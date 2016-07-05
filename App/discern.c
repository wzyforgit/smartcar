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

#define high_speed   0x01
#define median_speed 0x02
#define low_speed    0x03
static int32 mode=0;

static void read_DIPswitch(void)
{
    gpio_init(PTC16,GPI,0);
    gpio_init(PTC17,GPI,0);
    gpio_init(PTC18,GPI,0);
    gpio_init(PTC19,GPI,0);
    
    mode=GPIO_GET_NBIT(4,PTC16);
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

#define start_line 23
#define end_line (CAMERA_H-1)
/*���ھ�ͷ��Ⱦ�����Խ�end_line��ǰ11��ƫ����������ľ��������ܹ����µ�����ͷ�����ý��࣬���Խ�����ƫ����*/
#define base_line (CAMERA_W/2)
#define edge_offset (base_line+15)

#if(start_line<0||end_line>=CAMERA_H||end_line<=start_line||base_line<0||base_line>=CAMERA_W)
#error start and end line set error
#endif

static traffic traffic_type=beeline;

static local_t* get_midline(pixel_t *image)
{
    static local_t mids[CAMERA_H]={0};
    /*��ȡ������Ϣ*/
    boundary_t left_edge,right_edge;
    left_edge=serch_left_edge(image,start_line,end_line,base_line+15);
    right_edge=serch_right_edge(image,start_line,end_line,base_line-15);
    traffic_type=beeline;
    /*������*/
    /*static flag_t f_start=1;
    
    if(is_start(image,CAMERA_H-1))
    {
        if(f_start==1)
        {
            led(LED0,LED_ON);
            traffic_type=beeline;
            return NULL;
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
    }*/
    
    register count_t count;
    count_t left_lost,right_lost;
    left_lost=right_lost=0;
    count_t staggered=0;
    /*��������*/
    for(count=start_line;count<=end_line;count++)
    {
        flag_t get_left=left_edge.done[count];
        flag_t get_right=right_edge.done[count];
        if(get_left&&get_right)//���ҵ�
        {
            if(right_edge.edge[count]>left_edge.edge[count])
            {
                mids[count]=(local_t)(left_edge.edge[count]+right_edge.edge[count])/2;
            }
            else
            {
                staggered++;
            }
        }
        else if(!get_left&&get_right)//��ʧ
        {
            mids[count]=(local_t)(right_edge.edge[count]-edge_offset/2);
            left_edge.edge[count]=mids[count]-edge_offset/2;
            if(mids[count]>=80)
            {
                mids[count]=0;
            }
            left_lost++;
        }
        else if(get_left&&!get_right)//�Ҷ�ʧ
        {
            mids[count]=(local_t)(left_edge.edge[count]+edge_offset/2);
            right_edge.edge[count]=mids[count]+edge_offset/2;
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
    
    /*�ϰ���*/
    /*static count_t f_obstacle=NO_OBSTACLE;
    static count_t obstacle_keep=0;
    f_obstacle=is_obstacle(image,start_line,end_line,mids);
    if(f_obstacle!=NO_OBSTACLE)
    {
        count_t total=0;
        for(count=start_line;count<=end_line;count++)
        {
            total+=mids[count];
        }
        if(f_obstacle==LEFT_OBSTACLE)//���ϰ�
        {
            total=total/(end_line-start_line+1)+8;
        }
        else//���ϰ�
        {
            total=total/(end_line-start_line+1)-8;
        }
        for(count=start_line;count<=end_line;count++)
        {
            mids[count]=total;
        }
        return mids;
    }
    else
    {
        if(obstacle_keep!=0&&obstacle_keep!=5)//���ڳ��ϰ�
        {
            obstacle_keep++;
            return NULL;
        }
        if(obstacle_keep==5)//�����ϰ�
        {
            obstacle_keep=0;
        }
        led(LED2,LED_OFF);
        led(LED3,LED_OFF);
    }*/
    
    /*ʮ�֣�ֱ�ߣ����*/
    /*count_t lost_diff=left_lost-right_lost;
    LCD_printf(0,110,"%5d",lost_diff);*/
    /*if(lost_diff>=20)//����Ƭ��ʧ
    {
        traffic_type=curve;
        for(count=start_line;count<=end_line;count++)
        {
            mids[count]-=15;
            if(mids[count]>=CAMERA_W)
            {
                mids[count]=0;
            }
        }
    }
    else if(lost_diff<=-20)//�Ҳ��Ƭ��ʧ
    {
        traffic_type=curve;
        for(count=start_line;count<=end_line;count++)
        {
            mids[count]+=15;
            if(mids[count]>=CAMERA_W)
            {
                mids[count]=CAMERA_W-1;
            }
        }
    }
    else
    {
        traffic_type=beeline;
    }*/
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
    LCD_points(line_site,end-start+1,RED);
}

static speed_t speed_choose(traffic choose)
{
    speed_t speed=0;
    switch(mode)
    {
        case high_speed:
        switch(choose)
        {
            case curve   :speed=1000;break;
            case beeline :speed=1000;break;
            case crossing:speed=1000;break;
            case obstacle:speed=1000;break;
            default      :speed=1000;break;
        }
        break;
        
        case median_speed:
        switch(choose)
        {
            case curve   :speed=1000;break;
            case beeline :speed=1000;break;
            case crossing:speed=1000;break;
            case obstacle:speed=1000;break;
            default      :speed=1000;break;
        }
        break;
        
        case low_speed:
        switch(choose)
        {
            case curve   :speed=1000;break;
            case beeline :speed=1000;break;
            case crossing:speed=1000;break;
            case obstacle:speed=1000;break;
            default      :speed=1000;break;
        }
        break;
        
        default:speed=1000;
    }
    return speed;
}

static local_t get_average_mid(local_t start,local_t end,local_t *mids,pixel_t *image)
{
    static const double weight[CAMERA_H]={
	3.36,3.32,3.28,3.24,3.2,3.16,3.12,3.08,3.04,3,
	2.96,2.92,2.88,2.84,2.8,2.76,2.72,2.68,2.64,2.6,
	2.56,2.52,2.48,2.44,2.4,2.36,2.32,2.28,2.24,2.2,
	2.16,2.12,2.08,2.04,2,1.96,1.92,1.88,1.84,1.8,
	1.76,1.72,1.68,1.64,1.6,1.56,1.52,1.48,1.44,1.4,
	1.36,1.32,1.28,1.24,1.2,1.16,1.12,1.08,1.04,1,
    };//y=0.04x+1
    
    register count_t count;
    double mid_result;
    double div;
    mid_result=(mids[end]*weight[end]+mids[end-1]*weight[end-1]);
    div=weight[end]+weight[end-1];
    for(count=end-2;count>=start;count--)
    {
        /*if((image[mids[count]]==BLACK&&image[mids[count-1]]==BLACK&&image[mids[count-2]]==BLACK) ||
           (abs(mids[count]-mids[count+1])>=7&&abs(mids[count+1]-mids[count+2])>=7))
        {
            break;
        }*/
        mid_result+=(mids[count]*weight[count]);
        div+=weight[count];
    }
    display_lines(count+1,end,mids);//��ʾ����
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

static discern_result_t compute_result(local_t *mids,pixel_t *image)
{
    discern_result_t result={0,0};
    
    result.speed=speed_choose(traffic_type);
    five_point_smooth(start_line,end_line,mids);
    result.angle=get_average_mid(start_line,end_line,mids,image)-base_line;
    return result;
}

discern_result_t discern(void)
{
    camera_get_img();//��ȡͼ��
    
    img_extract(img, imgbuff, CAMERA_SIZE);//��ѹΪ�Ҷ�ͼ��

    pixel_t *new_img=NULL;
    new_img=image_adjust(img);
    
    LCD_Img_gray((Site_t){0, 0}, (Size_t){CAMERA_W, CAMERA_H}, new_img);
    local_t *mids;//����
    
    mids=get_midline(new_img);
    
    static discern_result_t result={0,300};//��ʼƫ�ǣ���ʼ�ٶ�

    if(mids==NULL)
    {
        return result;
    }

    result=compute_result(mids,new_img);//����ƫ�ǣ��ٶ�ѡ��
    
    return result;//����ʶ����
}