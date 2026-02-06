/*
 * GL08 双通道控制板硬件驱动实现
 * 包含 GPIO, ADC, PWM 和 Timer 硬件初始化和操作函数
 */

#include "STC8H.h"
#include "gl08_hardware.h"
#include "uart.h"

// Global variables
volatile uint8_t band_switch_1_pos = 0;  // Band switch 1 position
volatile uint8_t band_switch_2_pos = 0;  // Band switch 2 position
volatile uint8_t power_switch_pos = 0; // Power switch position

volatile uint16_t adc_values[3] = {0, 0, 0}; // ADC sampling values

//全局变量，存放输入捕获的占空比值
volatile uint16_t pwm1_duty = 0;   //PWM1，端口P1.0
volatile uint16_t pwm2_duty = 0;   //PWM2，端口P1.4

//用于计算占空比的临时变量
static uint16_t pwm1_rise_time = 0;
static uint16_t pwm1_fall_time = 0;
static uint16_t pwm2_rise_time = 0;
static uint16_t pwm2_fall_time = 0;

//辅助函数前向声明
static void bubble_sort(uint16_t arr[], uint8_t n);  //冒泡排序函数

//冒泡排序函数
static void bubble_sort(uint16_t arr[], uint8_t n) {
    uint8_t i, j;
    uint16_t temp;
    
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                // 交换元素
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/*========================================系统初始化及其相关操作函数=========================================*/

// System 初始化
void system_init(void) {
	
  // 失能硬件看门狗
  WDT_CONTR = 0x00;

}

// Hardware initialization
void hardware_init(void) {
  gpio_init();
  adc_init();
  pwm_init();
  timer_init();
}

// GPIO 初始化
void gpio_init(void) {

  // Set P1 port mode
  P1M1 |= (1<<0);  //10，模式2，高阻输入，P1.0端口配置，用于捕获第一路PWM1的输入
  P1M0 &= ~(1<<0);

  P1M1 |= (1<<1);  //10，模式2，高阻输入，P1.1端口配置，用于捕获第一路PWM2的输入
  P1M0 &= ~(1<<1);

  P1M1 |= (1<<4);  //10，模式2，高阻输入，P1.4端口配置，用于捕获第一路PWM2的输入，只有该端口能进行双通道捕获
  P1M0 &= ~(1<<4);

  // Set P3 port mode
  P3M1 |= (1<<2);  //10，模式2，高阻输入，P3.2端口配置，ADC采样，采集功率档位
  P3M0 &= ~(1<<2);

  P3M1 &= ~(1<<3);  //01，模式1，推挽输出，P3.3端口配置，第一路PWM输出
  P3M0 |= (1<<3);

  P3M1 &= ~(1<<4);  //01，模式1，推挽输出，P3.4端口配置，第二路PWM输出
  P3M0 |= (1<<4);

  P3M1 |= (1<<5);  //10，模式2，高阻输入，P3.5端口配置，ADC采样，采集第一路波段档位
  P3M0 &= ~(1<<5);

  P3M1 |= (1<<6);  //10，模式2，高阻输入，P3.6端口配置，ADC采样，采集第二路波段档位
  P3M0 &= ~(1<<6);

  // 初始化完后将PWM输出引脚拉低
  PWM_OUT1_CLR();
  PWM_OUT2_CLR();

}

/*========================================ADC初始化及其相关操作函数=========================================*/

// ADC 初始化
void adc_init(void) {

  ADCCFG = 0x2F;    //结果右对齐，时钟为16分频：SYSclk/2/16
  ADC_CONTR = 0x80; //使能 ADC 模块
  EADC = 0;         //失能 ADC 中断，采用查询的方式

  delay_ms(10);    //延时等待电源稳定

  // ADC_CONTR |= 0x4A; // 启动 ADC10 转换，P3.2 采集功率档位
  // ADC_CONTR |= 0x4D; // 启动 ADC13 转换，P3.5 采集波段旋钮1
  // ADC_CONTR |= 0x4E; // 启动 ADC14 转换, P3.6 采集波段旋钮2

}

// 转换 ADC 采样值为电压值，单位：mV
uint16_t adc_to_voltage(uint16_t adc_val) {
  // Assuming 5V reference voltage, 10-bit ADC
  return (uint16_t)((uint32_t)adc_val * 5000 / ADC_RESOLUTION);
}

// 启动 ADC 转换函数
uint16_t start_adc_conversion(uint8_t adc_chanel, uint8_t convert_count){

	uint16_t raw_value[3];
  uint8_t i, j;
  uint32_t sum = 0;
	
  // 检查采样次数是否足够
  if (convert_count < 3)
  {
    convert_count = 3; // 至少需要3次采样才能去掉两个最值
  }
	
  // 初始化数组
  for (i = 0; i < convert_count; i++)
  {
    raw_value[i] = 0;
  }

  for (i = 0; i < convert_count; i++)
  {
    ADC_CONTR &= 0xF0;                // 清除通道选择
    ADC_CONTR |= (0x0F & adc_chanel); // 选择通道
    ADC_CONTR |= 0x40;                // 启动转换
		


		for(j=0; j<250; j++)		//
		{
			if(ADC_CONTR & 0x20)
				{
				
					if (adc_chanel == (ADC_CONTR & 0x0F))
					{
						raw_value[i] = ((uint16_t)ADC_RES << 8) | ADC_RESL;
					}
					
					ADC_CONTR &= ~0x20; // 清除中断标志位
			
			  }

		}
 }
    
    bubble_sort(raw_value, convert_count);        // 对采样值进行排序
    
    // 去掉最小值和最大值，计算剩余值的平均值
    for (i = 1; i < convert_count - 1; i++) {    
        sum += raw_value[i];
    }
    
    return (uint16_t)(sum / (convert_count - 2));  // 返回平均值

}

// ADC 中断服务函数(这里未使用中断的方式，而是使用查询的方式)
void adc_Isr(void) interrupt 5 
{
  // 
}

/*========================================PWM初始化及其相关操作函数=========================================*/

// PWM 初始化
void pwm_init(void) {

  pwmb_oc_init(); //PWMB输出模式初始化

  pwma_ic_init(); //PWMA输入捕获模式初始化

}

// PWM比较输出初始化
void pwmb_oc_init(void){

  PWMB_PSCR = PWMB_PSC;  //24分频

  PWMB_PS = 0x50;  //bit7~bit4 = 0101，高级 PWM 通道 8 输出脚选择P3.4，bit7 bit6 = 01, 高级 PWM 通道 7 输出脚选择P3.3，bit5 bit4 = 01

  PWMB_CCER2 = 0x00;  //写 CCMRx 前必须先清零 CCxE 关闭通道
  PWMB_CCMR3 = 0x60;  //配置PWM7为PWM模式1
  PWMB_CCMR4 = 0x60;  //配置PWM8为PWM模式1

  PWMB_CCR7 = PWM7_DUTY;  //PWM7初始化占空比
  PWMB_CCR8 = PWM8_DUTY;  //PWM8初始化占空比

  PWMB_ARR = PWMB_PERIOD-1;  //PWMA周期

  PWMB_CCER2 = 0x11;  //使能PWM7、PWM8通道，高电平有效

  PWMB_ENO = 0x50;    //使能PWM7、PWM8端口输出
  PWMB_BKR = 0x80;    //使能主输出

  PWMB_CR1 = 0x01; //使能计数器

}

// PWM输入捕获初始化
void pwma_ic_init(void){

  PWMA_PSCR = PWMA_PSC;  //24分频，计数一个 TICK 为 1us
  PWMA_ARR = 0xFFFF;     // 最大计数周期

  PWMA_PS = 0x00;       //b5b4 = 00:PWM1P映射到P1.0；b1b0 = 00:PWM3P映射到P1.4

  PWMA_CCER1 = 0x00;   //bit0关闭CC1,bit4关闭CC2
  PWMA_CCMR1 = 0x01;   //IC1为输入模式，且映射到T11TP1上
  PWMA_CCMR1 &= 0x0F;	 // 滤波设为8个时钟
	PWMA_CCMR1 |= 0x30;	 // 
  PWMA_CCMR2 = 0x02;   //IC2为输入模式，且映射到T11TP2上
  PWMA_CCMR2 &= 0x0F;	 // 滤波设为8个时钟
	PWMA_CCMR2 |= 0x30;	
//  PWMA_CCER1 = 0x11;   //使能CC1,CC2上的输入捕获功能，通过外部函数再开启
  PWMA_CCER1 |= 0x00;  //设置捕获极性为CC1的上升沿
  PWMA_CCER1 |= 0x20;  //设置捕获极性为CC2的下降沿
  // PWMA_SMCR = 0X54; //TI1FP1(CC1映射在TI1FP1)从模式复位：上升沿复位计数器，即CC1通道捕获到上升沿会复位计数器

  PWMA_CCER2 = 0x00;   //bit0关闭CC3,bit4关闭CC4
  PWMA_CCMR3 = 0x01;   //IC3为输入模式，且映射到TI3TP3上
  PWMA_CCMR3 &= 0x0F;	 // 滤波设为8个时钟
	PWMA_CCMR3 |= 0x30;	 // 
  PWMA_CCMR4 = 0x02;   //IC4为输入模式，且映射到TI3TP4上
  PWMA_CCMR4 &= 0x0F;	 // 滤波设为8个时钟
	PWMA_CCMR4 |= 0x30;	 // 
//  PWMA_CCER2 = 0x11;   //使能CC3,CC4上的输入捕获功能，通过外部函数再开启
  PWMA_CCER2 |= 0x00;  //设置捕获极性为CC3的上升沿
  PWMA_CCER2 |= 0x20;  //设置捕获极性为CC4的下降沿

//  PWMA_IER = 0x1E; //使能CC1,CC2,CC3,CC4捕获中断，通过外部函数再开启

  PWMA_CR1 = 0x01; //使能计数器

}

// 动态调节PWM占空比
void set_pwm_duty(uint8_t channel, uint16_t duty)
{
  if (duty > PWM_FREQUENCY)
  {
    duty = PWM_FREQUENCY;
  }

  switch (channel)
  {
  case D1:
    PWMB_CCR7 = duty;  // duty 范围 0 ~ PWMB_ARR
    break;

  case D2:
    PWMB_CCR8 = duty;  // duty 范围 0 ~ PWMB_ARR
    break;
  
  default:

    break;
  }
}

// 动态获取输入捕获到的占空比值
uint16_t get_pwm_ic_duty(uint8_t channel){

  if (channel == PWM1)
  {
    return pwm1_duty;
  }

    return pwm2_duty;

}

// 开始 CC1 和 CC2 双通道捕获，同时捕获P1.0引脚(PWM1)
void pwma_ic1_start(void)
{
  PWMA_CCER1 |= 0x11; // 使能CC1,CC2输入捕获
  PWMA_IER |= 0x06;   // 使能捕获中断
  PWMA_CR1 |= 0x01;  // 确保计数器运行
}

// 开始 CC3 和 CC4 双通道捕获，同时捕获P1.4引脚(PWM2)
void pwma_ic2_start(void)
{
  PWMA_CCER2 |= 0x11; // 使能CC3,CC4输入捕获
  PWMA_IER |= 0x18;   // 使能捕获中断
  PWMA_CR1 |= 0x01;  // 确保计数器运行
}

// 停止捕获 PWM1
void pwma_ic1_stop(void)
{
  PWMA_CCER1 &= ~0x11; // 关闭CC1,CC2输入捕获
  PWMA_IER &= ~0x06;   // 关闭捕获中断
                     // 注意：这里不停止计数器，因为可能其他功能还在使用
}

// 停止捕获 PWM2
void pwma_ic2_stop(void)
{
	
  PWMA_CCER2 &= ~0x11; // 关闭CC3,CC4输入捕获
  PWMA_IER &= ~0x18;   // 关闭捕获中断
                     // 注意：这里不停止计数器，因为可能其他功能还在使用
}

// PWM 输入捕获中断服务函数
void pwm_ic_isr(void) interrupt 26  
{

	
    // 捕获PWM1
    if (PWMA_SR1 & 0x02)  // CC1上升沿捕获
    {
        pwm1_rise_time = PWMA_CCR1;
        PWMA_SR1 &= ~0x02;   // 清除中断标志位
    }
    if (PWMA_SR1 & 0x04)  // CC2下降沿捕获
    {
        pwm1_fall_time = PWMA_CCR2;
			  DISABLE_TIMER0();
        
        // 计算高电平时间，周期不变，且PWM分频系数一样，高电平时间即为占空比
        if (pwm1_fall_time >= pwm1_rise_time) {
            pwm1_duty = pwm1_fall_time - pwm1_rise_time;
        } else {
            pwm1_duty = (0xFFFF - pwm1_rise_time) + pwm1_fall_time;  //考虑计数器溢出
        }
        
        PWMA_SR1 &= ~0x04;   // 清标志
    }

    // 捕获PWM2
    if (PWMA_SR1 & 0x08)   // CC3上升沿捕获
    {
        pwm2_rise_time = PWMA_CCR3;
        PWMA_SR1 &= ~0x08;
    }
    if (PWMA_SR1 & 0x10)  // CC4下降沿捕获
    {
        pwm2_fall_time = PWMA_CCR4;
			  DISABLE_TIMER1();
        
        // 计算高电平时间，周期不变，且PWM分频系数一样，高电平时间即为占空比
        if (pwm2_fall_time >= pwm2_rise_time) {
            pwm2_duty = pwm2_fall_time - pwm2_rise_time;
        } else {
            pwm2_duty = (0xFFFF - pwm2_rise_time) + pwm2_fall_time;  //考虑计数器溢出
        }

        PWMA_SR1 &= ~0x10;				
    }
}

/*========================================Timer初始化及其相关操作函数=========================================*/

// Timer 初始化
void timer_init(void) {
	
  // 配置 Timer0 and Timer1 为 2ms 触发溢出中断，用于检测 PWM 0% 和 100% 的输入情况
  TMOD = 0x00; // Timer0 and Timer1 both works in mode 0  
  TH0 = TIMER0_RELOAD_H;
  TL0 = TIMER0_RELOAD_L;

  // Enable Timer0 interrupt
  ET0 = 1;

  // Start Timer0
  TR0 = 1;
	
	TL1 = TIMER1_RELOAD_H;
	TH1 = TIMER1_RELOAD_L; 

	// Enable Timer1 interrupt
	ET1 = 1; 
	
	// Start Timer1
	TR1 = 1; 
	
}

// Timer0 中断服务函数
void timer0_isr(void) interrupt 1
{
    TF0 = 0;  // 清除定时器0溢出中断标志
    
    // 在中断中关闭定时器0
    DISABLE_TIMER0(); 
	
    if(READ_PWM1_INPUT()){  //检测PWM1输入引脚P1.0电平值
        pwm1_duty = 1000;
    } else {
        pwm1_duty = 0;
    }

}

// Timer1 中断服务函数
void Timer1_ISR(void) interrupt 3 
{
    TF1 = 0;  // 清除定时器1溢出中断标志
    
    // 在中断中关闭定时器1
    DISABLE_TIMER1(); 
		
    if(READ_PWM2_INPUT()){  //检测PWM2输入引脚P1.4电平值
        pwm2_duty = 1000;
    } else {
        pwm2_duty = 0;
    }
}

/*========================================旋钮档位相关操作函数=========================================*/

// 读取波段旋钮档位
uint8_t read_band_switch(uint8_t band_switch) {

  uint8_t switch_state = 0;
  uint16_t adc_result = 0;
  uint16_t voltage_result = 0;  //转换为电压的结果单位为：mv
  adc_result = adc_values[band_switch];    // 读取存放的 ADC 采样值
  voltage_result = adc_to_voltage(adc_result);  // 将采样值转换为电压值（mv）

	// 根据电压值判断档位
  if (voltage_result <= (BAND_SWITCH_EXT_VOLGATE+OLLOW_VOLGATE_DIFFERENCE_VALUE))
  {
    switch_state = BAND_EXT;
  }
  else if (voltage_result >= (BAND_SWITCH_1_VOLGATE-OLLOW_VOLGATE_DIFFERENCE_VALUE) && 
            voltage_result <= (BAND_SWITCH_1_VOLGATE+OLLOW_VOLGATE_DIFFERENCE_VALUE))
  {
    switch_state = BAND_0;
  }
  else if (voltage_result >= (BAND_SWITCH_2_VOLGATE-OLLOW_VOLGATE_DIFFERENCE_VALUE) && 
            voltage_result <= (BAND_SWITCH_2_VOLGATE+OLLOW_VOLGATE_DIFFERENCE_VALUE))
  {
    switch_state = BAND_25;
  }
  else if (voltage_result >= (BAND_SWITCH_3_VOLGATE-OLLOW_VOLGATE_DIFFERENCE_VALUE) && 
            voltage_result <= (BAND_SWITCH_3_VOLGATE+OLLOW_VOLGATE_DIFFERENCE_VALUE))
  {
    switch_state = BAND_50;
  }
  else if (voltage_result >= (BAND_SWITCH_4_VOLGATE-OLLOW_VOLGATE_DIFFERENCE_VALUE) && 
            voltage_result <= (BAND_SWITCH_4_VOLGATE+OLLOW_VOLGATE_DIFFERENCE_VALUE))
  {
    switch_state = BAND_75;
  }
  else if (voltage_result >= (BAND_SWITCH_5_VOLGATE-OLLOW_VOLGATE_DIFFERENCE_VALUE) && 
            voltage_result <= BAND_SWITCH_5_VOLGATE)
  {
    switch_state = BAND_100;
  }
	
	if(band_switch == 0){
		
		uart_sendstr("band1 voltage(mv):");
		uart_uint16(voltage_result);
		uart_sentEnter();
	

	}
	else{
		uart_sendstr("band2 voltage(mv):");
		uart_uint16(voltage_result);
		uart_sentEnter();
		
	}

  return switch_state;
}

// 读取功率旋钮档位
uint8_t read_power_switch(void) {

  uint8_t switch_state = 0;
  uint16_t adc_result = 0;
  uint16_t voltage_result = 0;  //转换为电压的结果单位为：mv
  adc_result = adc_values[2];   // 读取存放的 ADC 采样值
  voltage_result = adc_to_voltage(adc_result);   // 将采样值转换为电压值（mv）

	// 根据电压值判断档位
  if (voltage_result <= (POWER_SWITCH_1_VOLGATE+OLLOW_VOLGATE_DIFFERENCE_VALUE))
  {
    switch_state = POWER_LIMIT_67;
  }
  else if (voltage_result >= (POWER_SWITCH_2_VOLGATE-OLLOW_VOLGATE_DIFFERENCE_VALUE) && 
          voltage_result <= (POWER_SWITCH_2_VOLGATE+OLLOW_VOLGATE_DIFFERENCE_VALUE))
  {
    switch_state = POWER_LIMIT_83;
  }
  else if (voltage_result >= (POWER_SWITCH_3_VOLGATE-OLLOW_VOLGATE_DIFFERENCE_VALUE) && 
          voltage_result <= POWER_SWITCH_3_VOLGATE+OLLOW_VOLGATE_DIFFERENCE_VALUE)
  {
    switch_state = POWER_LIMIT_100;
  }
	
	uart_sendstr("power voltage(mv):");
	uart_uint16(voltage_result);
	uart_sentEnter();
	
	uart_sentEnter();

  return switch_state;
}

// 扫描各个旋钮的档位
void scan_switches(void) {
  // Read band switch position
  band_switch_1_pos = read_band_switch(0);  // 扫描波段旋钮1（K1）的档位
  band_switch_2_pos = read_band_switch(1);  // 扫描波段旋钮2（K2）的档位

  // Read power switch position
  power_switch_pos = read_power_switch();   // 扫描功率旋钮的档位
	
	uart_sendstr("band1 switch pos:");
	uart_uint8(band_switch_1_pos);
	uart_sentEnter();
	
	uart_sendstr("band2 switch pos:");
	uart_uint8(band_switch_2_pos);
	uart_sentEnter();
	
	uart_sendstr("power switch pos:");
	uart_uint8(power_switch_pos);
	uart_sentEnter();
	
	uart_sentEnter();
	
}