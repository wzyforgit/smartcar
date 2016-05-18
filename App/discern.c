#include "include.h"
#include "algorithm.h"
#include <string.h>

/*����ͷӲ��*/
/*warning:����ɽ����BUG����ʹ������ͷ��ʱ���������κα������Ż�*/

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

static traffic traffic_type;

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

static void display_start_end(void);
void discern_init(void)
{
    camera_init(imgbuff);
    set_vector_handler(PORTA_VECTORn , PORTA_IRQHandler);   //����ͷ���ж�
    set_vector_handler(DMA0_VECTORn , DMA0_IRQHandler);     //����ͷDMA�ж�
    NVIC_SetPriority(PORTA_IRQn,0);
    NVIC_SetPriority(DMA0_IRQn,1);
    read_DIPswitch();
    display_start_end();
}

#define start_line 20
#define end_line (CAMERA_H-1)
#define base_line 38

#if(start_line<0||end_line>=CAMERA_H||end_line<=start_line||base_line<0||base_line>=CAMERA_W)
#error start and end line set error
#endif

static local_t* get_midline(pixel_t *image)
{
    static local_t mids[CAMERA_H]={0};
    
    /*��ȡ������Ϣ*/
    boundary_t left_edge,right_edge;
    left_edge=serch_left_edge(image,start_line,end_line,base_line+5);
    right_edge=serch_right_edge(image,start_line,end_line,base_line-5);
    
    /*ɨ��ǰ����*/
#if(start_line<=15)
    if(left_edge.done[start_line]+left_edge.done[start_line+1]+left_edge.done[start_line+2]+left_edge.done[start_line+3]+left_edge.done[start_line+4]<=2 &&
       right_edge.done[start_line]+right_edge.done[start_line+1]+right_edge.done[start_line+2]+right_edge.done[start_line+3]+right_edge.done[start_line+4]<=2)
    {
        return NULL;
    }
#endif

    /*������*/
    static flag_t f_start=1;
    
    if(is_start(image,start_line,end_line))
    {
        if(f_start==1)
        {
            led(LED0,LED_ON);
            traffic_type=beeline;
            return mids;
        }
        else
        {
            set_motor(motor_back,0);
            DisableInterrupts;
            led(LED1,LED_ON);
            while(1);
        }
    }
    
    if(f_start==1)
    {
        led(LED0,LED_OFF);
        f_start=0;
    }
    
    register count_t count;
    count_t left_lost,right_lost;
    left_lost=right_lost=0;
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
            left_edge.edge[count]=mids[count]-base_line/2;
            if(mids[count]>=80)
            {
                mids[count]=0;
            }
            left_lost++;
        }
        else if(get_left&&!get_right)//�Ҷ�ʧ
        {
            mids[count]=(local_t)(left_edge.edge[count]+base_line/2);
            right_edge.edge[count]=mids[count]+base_line/2;
            if(mids[count]>=80)
            {
                mids[count]=79;
            }
            right_lost++;
        }
        else//ȫ��ʧ
        {
            left_lost++;
            right_lost++;
        }
    }
    
    /*·���ж�*/
    
    /*ʮ�֣�ֱ�ߣ����*/
    count_t lost_diff=left_lost-right_lost;
    if(lost_diff>=20)//����Ƭ��ʧ
    {
        count_t done=0;
        for(count=start_line;count<=start_line+5;count++)
        {
            local_t x=mids[count];
            if(x<20)
            {
                traffic_type=curve;
                break;
            }
            if(image[x-9]==BLACK&&image[x-10]==BLACK&&image[x-11]==BLACK)
            {
                if(image[x-17]==WHITE&&image[x-18]==WHITE&&image[x-19]==WHITE)
                {
                    done++;
                }
            }
        }
        if(done>=3)
        {
            least_square(end_line-10,end_line,
                         start_line,end_line,
                         mids
                         );
            traffic_type=crossing;
        }
    }
    else if(lost_diff<=-20)//�Ҳ��Ƭ��ʧ
    {
        count_t done=0;
        for(count=start_line;count<=start_line+5;count++)
        {
            local_t x=mids[count];
            if(x>60)
            {
                traffic_type=curve;
                break;
            }
            if(image[x+9]==BLACK&&image[x+10]==BLACK&&image[x+11]==BLACK)
            {
                if(image[x+17]==WHITE&&image[x+18]==WHITE&&image[x+19]==WHITE)
                {
                    done++;
                }
            }
        }
        if(done>=3)
        {
            least_square(end_line-10,end_line,
                         start_line,end_line,
                         mids
                         );
            traffic_type=crossing;
        }
    }
    else
    {
        traffic_type=beeline;
    }
    return mids;
}

static void display_start_end(void)
{
    Site_t lines[20];
    register count_t count;
    for(count=CAMERA_W;count<CAMERA_W+20;count++)
    {
        lines[count-CAMERA_W].x=count;
        lines[count-CAMERA_W].y=start_line-1;
    }
    LCD_points(lines,20,BLUE);
    for(count=CAMERA_W;count<CAMERA_W+20;count++)
    {
        lines[count-CAMERA_W].x=count;
        lines[count-CAMERA_W].y=end_line+1;
    }
    LCD_points(lines,20,BLUE);
}

static void display_lines(local_t start,local_t end,local_t *lines)
{
    Site_t line_site[end-start+1];
    register count_t count,line_count;
    for(count=start,line_count=0;count<=end;count++,line_count++)
    {
        line_site[line_count].x=lines[count];
        line_site[line_count].y=count;
    }
    LCD_points(line_site,count,RED);
}

static speed_t speed_choose(traffic choose)
{
    speed_t speed=0;
    switch(mode)
    {
        case high_speed:
        switch(choose)
        {
            case curve   :speed=100;break;
            case beeline :speed=100;break;
            case crossing:speed=100;break;
            default      :speed=100;break;
        }
        break;
        
        case low_speed:
        switch(choose)
        {
            case curve   :speed=100;break;
            case beeline :speed=100;break;
            case crossing:speed=100;break;
            default      :speed=100;break;
        }
        break;
        
        default:speed=100;
    }
    return speed;
}

local_t get_average_mid(local_t start,local_t end,local_t *mids)
{
    static const double weight[CAMERA_H]={
	1,1.04,1.08,1.12,1.16,1.2,1.24,1.28,1.32,1.36,1.4,
	1.44,1.48,1.52,1.56,1.6,1.64,1.68,1.72,1.76,1.8,
	1.84,1.88,1.92,1.96,2,2.04,2.08,2.12,2.16,2.2,
	2.24,2.28,2.32,2.36,2.4,2.44,2.48,2.52,2.56,2.6,
	2.64,2.68,2.72,2.76,2.8,2.84,2.88,2.92,2.96,3,
	3.04,3.08,3.12,3.16,3.2,3.24,3.28,3.32,3.36,
    };
    
    register count_t count;
    double mid_result=0;
    double div=0;
    for(count=start;count<=end;count++)
    {
        mid_result+=(mids[count]*weight[count]);
        div+=weight[count];
    }
    
    double result=mid_result/div+0.5;
    if(result<0)
    {
        return 0;
    }
    else if(result>CAMERA_W-1)
    {
        return CAMERA_W-1;
    }
    else
    {
        return (local_t)result;
    }
}

static discern_result_t compute_result(local_t *mids)
{
    discern_result_t result={0,0};
    
    result.speed=speed_choose(traffic_type);
    five_point_smooth(start_line,end_line,mids);
    result.angle=get_average_mid(start_line,end_line,mids)-base_line;
    return result;
}

discern_result_t discern(void)
{
    camera_get_img();//��ȡͼ��
    
    img_extract(img, imgbuff, CAMERA_SIZE);//��ѹΪ�Ҷ�ͼ��
    
    LCD_Img_gray((Site_t){0, 0}, (Size_t){CAMERA_W, CAMERA_H}, img);//��ʾԭʼͼ��
    
    local_t *mids;//����
    
    mids=get_midline(img);//ʹ��ԭʼͼ��
    
    static discern_result_t result={0,0};
#if(start_line<=15)
    if(mids==NULL)
    {
        return result;
    }
#endif
    result=compute_result(mids);//����ƫ�ǣ��ٶ�ѡ��
    
    display_lines(start_line,end_line,mids);//��ʾ��Ч������
    return result;//����ʶ����
}