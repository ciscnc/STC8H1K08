/*
 * GL08 双通道控制板控制逻辑实现
 * 包含调光控制、模式切换等核心控制功能
 */
#include "type_def.h"
#include "gl08_control.h"
#include "gl08_hardware.h"
#include "uart.h"

// Global control data
control_data_t control_data[2];    //双通道，两个单独的控制结构体

// 辅助函数前向声明
uint16_t culculate_absolute_value(uint16_t value1, uint16_t value2);   // 计算两个无符号16位数的绝对值

// 控制逻辑结构体初始化
void control_init(void) {
  // control_data[0]控制第一路，链路为K1(波段旋钮1、ADC采样)、PWM1(输入捕获)、D1(PWM输出)；功率旋钮为共用
  control_data[0].channel_value = 0;
  control_data[0].control_mode = CONTROL_MODE_EXT;
  control_data[0].power_limit = POWER_LIMIT_100;
  control_data[0].band_position = BAND_EXT;
	
	// control_data[1]控制第一路，链路为K2(波段旋钮2、ADC采样)、PWM2(输入捕获)、D2(PWM输出)；功率旋钮为共用
	control_data[1].channel_value = 0;
  control_data[1].control_mode = CONTROL_MODE_EXT;
  control_data[1].power_limit = POWER_LIMIT_100;
  control_data[1].band_position = BAND_EXT;
	
}

// 处理外部输入的 PWM 信号（波段档位为 EXT 时）
void process_external_pwm_inputs(void){

  static uint16_t raw_pwm1_duty = 0;
  static uint16_t raw_pwm2_duty = 0;

  static uint16_t last_raw_pwm1_duty = 0;
  static uint16_t last_raw_pwm2_duty = 0;

  static uint16_t current_pwm1_duty = 0;
  static uint16_t current_pwm2_duty = 0;

  static uint16_t previous_pwm1_duty = 0;
  static uint16_t previous_pwm2_duty = 0;

  static uint8_t pwm1_same_value_count= 0;
  static uint8_t pwm2_same_value_count= 0;

  static uint8_t pwm1_level_off_flag = 0;
  static uint8_t pwm2_level_off_flag = 0;

  static uint8_t pwm1_filter_count = 0;
  static uint8_t pwm2_filter_count = 0;

	// 通道1为外部处理模式
  if (control_data[0].control_mode == CONTROL_MODE_EXT)
  {
    raw_pwm1_duty = get_pwm_ic_duty(PWM1);   // 获取 PWM1 的占空比（未处理值）

    if (culculate_absolute_value(raw_pwm1_duty, current_pwm1_duty) >= 50)   //认定为新值，外部发生档位变化
    {
      current_pwm1_duty = raw_pwm1_duty;
      pwm1_level_off_flag = 0;
    }
    else if(pwm1_level_off_flag == 0)
    {
      if (culculate_absolute_value(raw_pwm1_duty, last_raw_pwm1_duty) == 0)
      {
        pwm1_same_value_count++;
      }
      else
        pwm1_same_value_count = 0;

      if (pwm1_same_value_count >= 3) // 连续捕获三次相同值，认为已达到平稳转态，直接按捕获值输出
      {
        current_pwm1_duty = raw_pwm1_duty;
        pwm1_level_off_flag = 1;
      }

      if (pwm1_level_off_flag == 0)  // 开始滤波算法
      {
        uint16_t difference_value1;
        uint16_t difference_value2;

        difference_value1 = culculate_absolute_value(current_pwm1_duty, raw_pwm1_duty);
        difference_value2 = culculate_absolute_value(current_pwm1_duty, previous_pwm1_duty);

        if (pwm1_filter_count <= 10)
        {
          if (difference_value1 < 10)   //刚开始采样，去除掉不合理值后，将采样值置为置信度更高的输入源
          {
            current_pwm1_duty = 0.4*current_pwm1_duty + 0.6*raw_pwm1_duty;
          }
          else {

            current_pwm1_duty = current_pwm1_duty;
            previous_pwm1_duty = previous_pwm1_duty;
          }
          
          pwm1_filter_count++;
        }
        else{

          if (difference_value1 > difference_value2)  //当前值与采样值波动较大，采用先前值滤波
          {
            current_pwm1_duty = 0.5*current_pwm1_duty + 0.5*previous_pwm1_duty;
          }
          else current_pwm1_duty = 0.5*current_pwm1_duty + 0.5*raw_pwm1_duty;  //否则采用采样值滤波

          previous_pwm1_duty = current_pwm1_duty;
          
        }
        
      }

    }

    last_raw_pwm1_duty = raw_pwm1_duty; 
		
		// 串口调试，可在gl08_config.h中通过宏 UART_PRINT 开启或关闭
		uart_sendstr("PWM1 input raw value:");
		uart_uint16(raw_pwm1_duty);
		uart_sentEnter();
		
		uart_sendstr("PWM1 input filter value:");
		uart_uint16(control_data[0].channel_value);
		uart_sentEnter();
		
		uart_sentEnter();
  }

	// 通道1为外部处理模式
  if (control_data[1].control_mode == CONTROL_MODE_EXT)
  {
    raw_pwm2_duty = get_pwm_ic_duty(PWM2);    // 获取 PWM2 的占空比（未处理值）

    if (culculate_absolute_value(raw_pwm2_duty, current_pwm2_duty) >= 100)   //认定为新值，外部发生档位变化
    {
      // last_raw_pwm2_duty = raw_pwm2_duty;
      current_pwm2_duty = raw_pwm2_duty;
      // previous_pwm2_duty = raw_pwm2_duty;
      pwm2_level_off_flag = 0;
    }
    else if(pwm2_level_off_flag == 0)
    {
      if (culculate_absolute_value(raw_pwm2_duty, last_raw_pwm2_duty) == 0)
      {
        pwm2_same_value_count++;
      }
      else
        pwm2_same_value_count = 0;

      if (pwm2_same_value_count >= 3) // 连续捕获三次相同值，认为已达到平稳转态，直接按捕获值输出
      {
        current_pwm2_duty = raw_pwm2_duty;
        pwm2_level_off_flag = 1;
      }

      if (pwm2_level_off_flag == 0)  // 开始滤波算法
      {
        uint16_t difference_value1;
        uint16_t difference_value2;

        difference_value1 = culculate_absolute_value(current_pwm2_duty, raw_pwm2_duty);
        difference_value2 = culculate_absolute_value(current_pwm2_duty, previous_pwm2_duty);

        if (pwm2_filter_count <= 10)
        {
          if (difference_value1 < 10)   //刚开始采样，去除掉不合理值后，将采样值置为置信度更高的输入源
          {
            current_pwm2_duty = 0.4*current_pwm2_duty + 0.6*raw_pwm2_duty;
          }
          else {

            current_pwm2_duty = current_pwm2_duty;
            previous_pwm2_duty = previous_pwm2_duty;
          }
          
          pwm2_filter_count++;
        }
        else{

          if (difference_value1 > difference_value2)  //当前值与采样值波动较大，采用先前值滤波
          {
            current_pwm2_duty = 0.5*current_pwm2_duty + 0.5*previous_pwm2_duty;
          }
          else current_pwm2_duty = 0.5*current_pwm2_duty + 0.5*raw_pwm2_duty;  //否则采用采样值滤波

          previous_pwm2_duty = current_pwm2_duty;
          
        }
        
      }

    }
     
    last_raw_pwm2_duty = raw_pwm2_duty; 
		
		// 串口调试，可在gl08_config.h中通过宏 UART_PRINT 开启或关闭
		uart_sendstr("PWM2 input raw value:");
		uart_uint16(raw_pwm2_duty);
		uart_sentEnter();
		
		uart_sendstr("PWM2 input filter value:");
		uart_uint16(control_data[1].channel_value);
		uart_sentEnter();
		
		uart_sentEnter();
  }
  // External panel control mode
  control_data[0].channel_value = current_pwm1_duty;   // 更新当前占空比值到控制字
  control_data[1].channel_value = current_pwm2_duty;
	
}

// 处理获取到的波段档位信息
void process_band_switch(void) {
	
  control_data[0].band_position= band_switch_1_pos;
  control_data[1].band_position = band_switch_2_pos;

  // 基于波段档位1（K1）设置通道1的控制模式
  if (control_data[0].band_position  == BAND_EXT) {
    control_data[0].control_mode = CONTROL_MODE_EXT;
    pwma_ic1_start();
    EABLE_TIMER0();
  } else {
    control_data[0].control_mode = CONTROL_MODE_LOCAL;
		pwma_ic1_stop();
    DISABLE_TIMER0();
  }

  // 基于波段档位2（K2）设置通道2的控制模式
  if (control_data[1].band_position == BAND_EXT) {
    control_data[1].control_mode = CONTROL_MODE_EXT;
    pwma_ic2_start();
    EABLE_TIMER1();
  } else {
    control_data[1].control_mode = CONTROL_MODE_LOCAL;
		pwma_ic2_stop();
    DISABLE_TIMER1();
  }
	
	// 串口调试，可在gl08_config.h中通过宏 UART_PRINT 开启或关闭
	uart_sendstr("band1 control mode:");
	uart_uint8(control_data[0].control_mode);
	uart_sentEnter();
	
	uart_sendstr("band2 control mode:");
	uart_uint8(control_data[1].control_mode);
	uart_sentEnter();
	
	uart_sentEnter();
}

// 处理获取到的功率档位信息
void process_power_switch(void) { 

  control_data[0].power_limit = power_switch_pos; 
  control_data[1].power_limit = power_switch_pos;
	
	// 串口调试，可在gl08_config.h中通过宏 UART_PRINT 开启或关闭
	uart_sendstr("power_limit:");
	uart_uint8(control_data[0].power_limit);
	uart_sentEnter();
	
	uart_sentEnter();
}

// 更新通道输出
void update_outputs(void) {

  uint16_t ch1_output, ch2_output;

  // Calculate output values
  ch1_output = calculate_output_value(GL08_CH1);
  ch2_output = calculate_output_value(GL08_CH2);

  // Apply power limit
  apply_power_limit(GL08_CH1,&ch1_output);
  apply_power_limit(GL08_CH2,&ch2_output);

  // Set outputs	
	set_pwm_duty(GL08_CH1, ch1_output);
	set_pwm_duty(GL08_CH2, ch2_output);
	
	// 串口调试，可在gl08_config.h中通过宏 UART_PRINT 开启或关闭
	uart_sendstr("ch1 output:");
	uart_uint16(ch1_output);
	uart_sentEnter();
	
	uart_sendstr("ch2 output:");
	uart_uint16(ch2_output);
	uart_sentEnter();
	
	uart_sendstr("====== output end ======\r\n");
	
	uart_sentEnter();


}

// 应用功率限制
void apply_power_limit(uint8_t gl08_channel, uint16_t *value) {
	
  uint8_t band_mode = get_control_mode(gl08_channel);

  if (band_mode == CONTROL_MODE_LOCAL)
  {
    switch (control_data[gl08_channel-1].power_limit)
    {
    case POWER_LIMIT_67:
      *value = (uint16_t)((uint32_t)(*value) * 667 / 1000);   // 档位1，66.7%
      break;
    case POWER_LIMIT_83:
      *value = (uint16_t)((uint32_t)(*value) * 833 / 1000);   // 档位2，88.3%
      break;
    case POWER_LIMIT_100:   // 档位3，100%
    default:
      // No power limit
      break;
    }
  }
}

// 设置通道控制模式
void set_control_mode(uint8_t gl08_channel, uint8_t mode){
	
	if (gl08_channel == GL08_CH1) {
	
		if (mode <= CONTROL_MODE_EXT) {
			control_data[0].control_mode = mode;
		}
	}
	else if (gl08_channel == GL08_CH2){
		
			if (mode <= CONTROL_MODE_EXT) {
			control_data[1].control_mode = mode;
		}
	}
}

// 获取通道控制模式
uint8_t get_control_mode(uint8_t gl08_channel) {
	
  uint8_t band_mode = 0;
	
  if (gl08_channel == GL08_CH1)
  {
    band_mode = control_data[0].control_mode;
  }
  else if (gl08_channel == GL08_CH2)
  {
    band_mode = control_data[1].control_mode;
  }
	
   return band_mode; 
}

// 计算通道PWM输出值
uint16_t calculate_output_value(uint8_t pwm_channel) {
	
  uint16_t output_value = 0;
  uint8_t band_mode = 0;

  band_mode = get_control_mode(pwm_channel);

  switch (band_mode) {
  case CONTROL_MODE_LOCAL:
    // Local band switch control
    output_value = apply_band_setting(pwm_channel, PWM_FREQUENCY); // Base PWM_FREQUENCY(1000) value
    break;

  case CONTROL_MODE_EXT:
    // External panel control
    if (pwm_channel == D1) {
      output_value = control_data[0].channel_value;
    } 
		else if(pwm_channel == D2){
      output_value = control_data[1].channel_value;
    }
    break;

  default:
    output_value = 0;
    break;
  }

  return output_value;
}

// Apply band setting
uint16_t apply_band_setting(uint8_t band_switch, uint16_t input_value) {

  static uint16_t output_value = 0;
  uint8_t band_poition = 0;

  if (band_switch == K1)
  {
    band_poition = control_data[0].band_position;
  }
  else if (band_switch == K2)
  {
    band_poition = control_data[1].band_position;
  }

  switch (band_poition) {
  case BAND_0:
    output_value = input_value * 0; // 0%
    break;

  case BAND_25:
    output_value = input_value / 4; // 25%
    break;

  case BAND_50:
    output_value = input_value / 2; // 50%
    break;

  case BAND_75:
    output_value = input_value * 3 / 4; // 75%
    break;

  case BAND_100:
    output_value = input_value; // 100%
    break;

  case BAND_EXT:
  default:
    output_value = output_value; // 档位未知，保持上次输出
    break;
  }

  return output_value;
}

// 开始 ADC 转换，采集各个旋钮（波段、功率）的档位信息
void collect_inputs(void){

	adc_values[ADC_CH1] = start_adc_conversion(BAND_SWITCH_1, CONSECUTIVE_CONV);
	adc_values[ADC_CH2] = start_adc_conversion(BAND_SWITCH_2, CONSECUTIVE_CONV);
	adc_values[ADC_CH3] = start_adc_conversion(POWER_SWITCH, CONSECUTIVE_CONV);

	// 串口调试，可在gl08_config.h中通过宏 UART_PRINT 开启或关闭
	uart_sendstr("band1 raw adc_value(0~1023):");
	uart_uint16(adc_values[ADC_CH1]);
	uart_sentEnter();
	
	uart_sendstr("band2 raw adc_value(0~1023):");
	uart_uint16(adc_values[ADC_CH2]);
	uart_sentEnter();
	
	uart_sendstr("power raw adc_value(0~1023):");
	uart_uint16(adc_values[ADC_CH3]);
	uart_sentEnter();
	
	uart_sentEnter();
}

// 计算两数的差值（绝对值）
uint16_t culculate_absolute_value(uint16_t value1, uint16_t value2){

  if (value1 > value2)
  {
    return value1 - value2;
  }

  return value2 -value1;
    
}
