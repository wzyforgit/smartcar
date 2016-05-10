#include "include.h"
#include "algorithm.h"
#include <string.h>

/*����ͷӲ��*/

static pixel_t imgbuff[CAMERA_SIZE];
static pixel_t img[CAMERA_H*CAMERA_W];

static void PORTA_IRQHandler(void)
{
    uint8  n;    //���ź�
    uint32 flag; //��Ҫ��Ϊʹ��flag_t

    while(!PORTA_ISFR);
    flag = PORTA_ISFR;
    PORTA_ISFR  = ~0;             //���жϱ�־λ

    n = 29;                       //���ж�
    if(flag & (1 << n))           //PTA29�����ж�
    {
        camera_vsync();
    }
#if ( CAMERA_USE_HREF == 1 )      //ʹ�����ж�
    n = 28;
    if(flag & (1 << n))           //PTA28�����ж�
    {
        camera_href();
    }
#endif
}

static void DMA0_IRQHandler(void)
{
    camera_dma();
}

/*ʶ��*/

#define high_speed 0x01
#define low_speed  0x02
static int32 mode=0;

static void read_DIPswitch(void)
{
    port_init(PTC16, ALT1 | PULLUP );
    GPIO_PDDR_REG(GPIOX_BASE(PTC16)) &= ~(1 << PTn(PTC16));
    
    port_init(PTC17, ALT1 | PULLUP );
    GPIO_PDDR_REG(GPIOX_BASE(PTC17)) &= ~(1 << PTn(PTC17));
    
    port_init(PTC18, ALT1 | PULLUP );
    GPIO_PDDR_REG(GPIOX_BASE(PTC18)) &= ~(1 << PTn(PTC18));
    
    port_init(PTC19, ALT1 | PULLUP );
    GPIO_PDDR_REG(GPIOX_BASE(PTC19)) &= ~(1 << PTn(PTC19));
    
    mode=GPIO_GET_NBIT(3,PTC16);
    LCD_printf(0,61,"%d",mode);
}

void discern_init(void)
{
    camera_init(imgbuff);
    set_vector_handler(PORTA_VECTORn , PORTA_IRQHandler);   //����ͷ���ж�
    set_vector_handler(DMA0_VECTORn , DMA0_IRQHandler);     //����ͷDMA�ж�
    NVIC_SetPriority(PORTA_IRQn,0);
    NVIC_SetPriority(DMA0_IRQn,1);
    read_DIPswitch();
}

#define start_line 5
#define end_line (CAMERA_H-5)
#define base_line 40

static local_t* get_midline(pixel_t *image)
{
    static local_t mids[CAMERA_H];
    
    boundary_t left_edge,right_edge;
    left_edge=serch_left_black_line(image,start_line,end_line,base_line+10);
    right_edge=serch_right_black_line(image,start_line,end_line,base_line-10);
    
    register count_t count;
    
    /*ɨ��ǰ����*/
    if(left_edge.done[start_line]+left_edge.done[start_line+1]+left_edge.done[start_line+2]+left_edge.done[start_line+3]+left_edge.done[start_line+4]<=2 &&
       right_edge.done[start_line]+right_edge.done[start_line+1]+right_edge.done[start_line+2]+right_edge.done[start_line+3]+right_edge.done[start_line+4]<=2)
    {
        goto end_of_get_midline;
    }
    /*���ߴ���*/
    
    /*��������*/
    for(count=start_line;count<=end_line;count++)
    {
        flag_t get_left=left_edge.done[count];
        flag_t get_right=right_edge.done[count];
        if(get_left&&get_right)//���ҵ�
        {
            mids[count]=(local_t)(left_edge.edge[count]+right_edge.edge[count])/2;
        }
        else if(!get_left&&get_right)//��ʧ
        {
            mids[count]=(local_t)(right_edge.edge[count]-base_line/2);
            if(mids[count]>=80)
            {
                mids[count]=0;
            }
        }
        else if(get_left&&!get_right)//�Ҷ�ʧ
        {
            mids[count]=(local_t)(left_edge.edge[count]+base_line/2);
            if(mids[count]>=80)
            {
                mids[count]=79;
            }
        }
        else//ȫ��ʧ
        {
            ;
        }
    }
    
end_of_get_midline:
    return mids;
}

static void display_lines(local_t start,local_t end,local_t *lines)
{
    Site_t line_site[CAMERA_H];
    register count_t count;
    for(count=start;count<=end;count++)
    {
        line_site[count].x=lines[count];
        line_site[count].y=count;
    }
    LCD_points(line_site+start,count,RED);
}

static speed_t speed_choose(traffic choose)
{
    speed_t speed=0;
    switch(mode)
    {
        case high_speed:
        switch(choose)
        {
            case curve   :speed=200;break;
            case beeline :speed=100;break;
            case crossing:speed=100;break;
            default      :speed=100;break;
        }
        break;
        
        case low_speed:
        switch(choose)
        {
            case curve   :speed=200;break;
            case beeline :speed=100;break;
            case crossing:speed=100;break;
            default      :speed=100;break;
        }
        break;
        
        default:speed=300;
    }
    return speed;
}

static discern_result_t compute_result(local_t *mids)
{
    discern_result_t result={0,0};
    
    result.speed=speed_choose(beeline);
    five_point_smooth(start_line,end_line,mids);
    result.angle=get_average_mid(start_line,end_line,mids)-base_line;
    return result;
}

discern_result_t discern(void)
{
    camera_get_img();//��ȡͼ��
    
    img_extract(img, imgbuff, CAMERA_SIZE);//��ѹΪ�Ҷ�ͼ��
    LCD_Img_gray((Site_t){0, 0}, (Size_t){CAMERA_W, CAMERA_H}, img);//��ʾͼ��
    
    local_t *mids;//����
    mids=get_midline(img);//����������������Ч���
    
    discern_result_t result={0,0};
    result=compute_result(mids);//����ƫ�ǣ��ٶ�ѡ��
    
    display_lines(start_line,end_line,mids);//��ʾ��Ч������
    return result;//����ʶ����
}