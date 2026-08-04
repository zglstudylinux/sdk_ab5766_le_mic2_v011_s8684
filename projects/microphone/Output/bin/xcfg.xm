depend(0x01020000)

config(SUB, "功能选择", "系统功能配置");
config(CHECK, "无线mic适配器", "是否打开无线mic接收适配器功能", WIRELESS_ADAPTER_EN, 0);
config(CHECK, "无线mic发射端", "是否打开无线mic发射端功能", WIRELESS_MIC_EMIT_EN, 0);

config(SUB, "发射端参数", "发射端参数配置", WIRELESS_MIC_EMIT_EN);
config(LISTVAL, "无连接时自动关机时间", "设置自动关机时间", SYS_OFF_TIME, 	SHORT, 0, 18, ("不关机", 0), ("30秒钟后", 30), ("1分钟后", 60),  ("2分钟后", 120), ("3分钟后", 180), ("4分钟后", 240), ("5分钟后", 300), ("6分钟后", 360), ("7分钟后", 420), ("8分钟后", 480), ("9分钟后", 540), ("10分钟后", 600), ("15分钟后", 900), ("20分钟后", 1200), ("25分钟后", 1500), ("30分钟后", 1800), ("45分钟后", 2700), ("1小时后", 3600), WIRELESS_MIC_EMIT_EN);
config(CHECK, "发射端第一次上电开机", "发射端第一次上电是否自动开机", POWERON_FIRST_BAT_EN, 1, WIRELESS_MIC_EMIT_EN);
config(LIST, "低电提示电压", "选择低电提示电压", LPWR_WARNING_VBAT_LEVEL, 11, "不提示", "2.8V", "2.9V", "3.0V", "3.1V", "3.2V", "3.3V", "3.4V", "3.5V", "3.6V", "3.7V", 0, WIRELESS_MIC_EMIT_EN);
config(LIST, "低电关机电压", "选择低电关机电压", LPWR_OFF_VBAT_LEVEL, 11, "不关机", "2.8V", "2.9V", "3.0V", "3.1V", "3.2V", "3.3V", "3.4V", "3.5V", "3.6V", "3.7V", 0, WIRELESS_MIC_EMIT_EN);

config(SUB, "系统配置", "系统相关配置");
config(LEVEL, 0x03);
config(CHECK, "BUCK MODE", "是否设置为BUCK MODE", BUCK_MODE_EN, 0);
config(CHECK, "VDDBT省电容", "VDDBT是否省电容，只能LDO模式才能省电容,BUCK模式下该配置无效", VDDBT_CAPLESS_EN, 1);
config(LISTVAL, "VDDIO电压", "选择VDDIO电压。", VDDIO_SEL, BIT, 4, 10, 7, ("2.7V", 8), ("2.85V", 9), ("3.0V", 10), ("3.15V", 11), ("3.3V", 12), ("3.45V", 13), ("3.6V", 14));
config(LISTVAL, "VDDBT电压", "选择VDDBT电压。", VDDBT_SEL, BIT, 4, 9, 5, ("1.1V", 5), ("1.2V", 7), ("1.3V", 9), ("1.4V", 11), ("1.5V", 13));
config(LISTVAL, "VDDCORE电压", "选择VDDCORE电压。", VDDCORE_SEL, BIT, 5, 20, 5, ("1.0V", 12), ("1.1V", 16), ("1.2V", 20), ("1.3V", 24), ("1.4V", 28));
config(LIST, "OSC基础电容", "同时配置24晶振OSCI与OSCO负载电容", UOSC_BOTH_CAP, 2, "0PF", "6PF", 0);
config(BYTE, "自定义OSCI电容", "配置24M晶振OSCI负载电容（0.25PF）。电容大小：n * 0.25PF + 基础电容", UOSCI_CAP, 0, 63, 45);
config(BYTE, "自定义OSCO电容", "配置24M晶振OSCO负载电容（0.25PF）。电容大小：n * 0.25PF + 基础电容", UOSCO_CAP, 0, 63, 45);
config(CHECK,  "优先使用产测电容值",  "是否优先使用产测校准的OSC电容值，没过产测或者关闭时使用<自定义OSCI电容>、<自定义OSCO电容>",  FT_OSC_CAP_EN,  1);
config(CHECK, "HUART调试(EQ/FCC)", "是否使用HUART调试功能", HUART_EN, 0);
config(LISTVAL, "HUART串口选择", "选择HUART的IO", HUART_IO_SEL, BIT, 8, 0xB3, 26, ("PA0", 0xA0), ("PA1", 0xA1), ("PA2", 0xA2), ("PA3", 0xA3), ("PA4", 0xA4), ("PA5", 0xA5), ("PA6", 0xA6), ("PA7", 0xA7), ("PA8", 0xA8), ("PA9", 0xA9), ("PA10", 0xAA),  ("PA11", 0xAB), ("PA12", 0xAC), ("PA13", 0xAD), ("PA14", 0xAE), ("PA15", 0xAF), ("PB0", 0xB0), ("PB1", 0xB1), ("PB2", 0xB2), ("PB3", 0xB3), ("PB4", 0xB4), ("PB5", 0xB5), ("PB6", 0xB6), ("PB7", 0xB7), ("PB8", 0xB8), ("PB9", 0xB9), HUART_EN)

config(SUB, "DAC配置", "DAC的相关配置");
config(LISTVAL, "DAC输出选择", "选择DAC的输出方式, ", DAC_SEL, BIT, 2, 0, 2,  ("单端输出", 0), ("差分输出", 1));

config(SUB, "充电配置", "充电功能的相关配置，需要打开宏BSP_CHARGE_EN");
config(CHECK, "充电使能", "是否打开充电功能", CHARGE_EN, 1);
config(CHECK, "涓流充电使能", "是否使能涓流充电", CHARGE_TRICK_EN, 1, CHARGE_EN);
config(CHECK, "插入DC复位", "DC IN时，复位", CHARGE_DC_IN_RST, 1, CHARGE_EN);
config(CHECK, "退出充电自动开机", "退出充电自动开机", POWERON_VUSB_OUT_EN, 1, CHARGE_EN);
config(CHECK, "边充电边工作使能", "需要关闭 插入DC禁止软开机", CHARGE_WORKING_WHILE_CHARGING, 0, CHARGE_EN);
config(LIST, "充电截止电流", "选择充电截止电流", CHARGE_STOP_CURR, 16, "0mA", "2.5mA", "5mA", "7.5mA", "10mA", "12.5mA", "15mA", "17.5mA", "20mA", "22.5mA", "25mA", "27.5mA", "30mA", "32.5mA", "35mA", "37.5mA", 12, CHARGE_EN);
config(LIST, "恒流充电电流", "恒流充电（电池电压大于2.9v）电流", CHARGE_CONSTANT_CURR, 33, "5mA", "10mA", "15mA", "20mA", "25mA", "30mA", "35mA", "40mA", "45mA", "50mA", "55mA", "60mA", "65mA", "70mA", "75mA", "80mA", "85mA", "90mA", "95mA", "100mA", "105mA", "110mA", "115mA", "120mA", "125mA", "130mA", "135mA", "140mA",, "145mA", "150mA", "155mA", "160mA", "165mA", 7, CHARGE_EN);
config(LIST, "涓流充电电流", "涓流充电（电池电压小于2.9v）电流", CHARGE_TRICKLE_CURR, 8, "5mA", "10mA", "15mA", "20mA", "25mA", "30mA", "35mA", "40mA", 3, CHARGE_TRICK_EN);
config(LISTVAL, "充电截止计时", "电压满足后开始计数", CHARGE_STOP_TIME, SHORT, 3, 13, ("不计时", 0), ("10min", 1), ("20min", 2),("30min", 3),("40min", 4),("50min", 5),("60min", 6),("70min", 7),("80min", 8),("90min", 9),("120min", 12),("150min", 15),("180min", 18);
config(LISTVAL, "充电仓类型", "选择充电仓类型", CH_BOX_TYPE, BIT, 2, 0, 3, ("关闭充电仓功能", 0), ("5V不掉电(常5V)", 1), ("5V掉电到维持电压", 2));
config(SUB, "蓝牙配置", "蓝牙的相关配置");
config(TEXT, "蓝牙名称", "无线麦连接的蓝牙名称", LE_NAME, 32, "LE_MIC2");
config(MAC, "蓝牙地址", "蓝牙的MAC地址", LE_ADDR, 6, 41:42:00:00:00:00, 41:42:FF:FF:FF:FF, 41:42:00:00:00:01);

config(SUB, "RF参数", "蓝牙RF参数");
config(LISTVAL, "RF TX功率选择", "选择RF的TX功率", RF_TX_PWR, BIT, 8, 0, 4, ("11dBm", 11), ("8dBm", 8), ("4dBm", 4), ("0dBm", 0));
config(CHECK, "自定义RF参数", "选用自定义RF参数后，RF TX功率选择无效", BT_RF_PARAM_EN, 0);
config(BYTE, "PA_GAIN", "PA_GAIN, 参考值3",  RF_PA_GAIN, 0,  7,  3, BT_RF_PARAM_EN);
config(BYTE, "PA_VSEL", "PA_VSEL, 参考值8",  RF_PA_VSEL, 0,  15, 8, BT_RF_PARAM_EN);
config(BYTE, "PA_MODE", "PA_MODE, 0:PA从VDDBT取电 1:PA从VDDIO取电", RF_PA_MODE, 0, 1, 1, BT_RF_PARAM_EN);

config(SUB, "MIC参数", "MIC参数配置", WIRELESS_MIC_EMIT_EN);
config(LISTVAL, "MIC偏置电路配置", "MICL电路BIAS配置，省电容，省电阻配置", MIC_BIAS_METHOD, BIT, 2, 1, 3, ("省RC电路", 0), ("单端外部电阻电容", 1), ("差分", 2), WIRELESS_MIC_EMIT_EN);
config(BYTE, "MIC_BIAS_RES", "MIC_BIAS电阻 = 16k/MIC BIAS RES，范围0-63", MIC_BIAS_RES, 0, 63, 6, WIRELESS_MIC_EMIT_EN);
config(BYTE, "MICM数字增益", "MICM数字增益配置", MIC_DIG_GAIN,  0, 36, 0, , WIRELESS_MIC_EMIT_EN);
config(LISTVAL, "MICM模拟增益", "MICM模拟增益配置，省rc默认8.5db增益", MIC_ANL_GAIN, BIT, 2, 1, 3, ("0db增益", 1), ("6db增益", 2), ("12db增益", 3), WIRELESS_MIC_EMIT_EN);

config(SUB, "按键配置", "选择按键及按键功能配置");
config(LISTVAL, "软开机长按时间选择", "长按PWRKEY多长时间后软开机", PWRON_PRESS_TIME,  BIT, 3, 3, 8, ("0.1秒", 0), ("0.5秒", 1), ("1秒", 2), ("1.5秒", 3), ("2秒", 4), ("2.5秒", 5), ("3秒", 6), ("3.5秒", 7));
config(LISTVAL, "软关机长按时间选择", "长按PWRKEY多长时间后软关机", PWROFF_PRESS_TIME, BIT, 3, 3, 8, ("1.5秒", 0), ("2秒", 1), ("2.5秒", 2), ("3秒", 3), ("3.5秒", 4), ("4秒", 5), ("4.5秒", 6), ("5秒", 7));
config(LISTVAL, "双击响应时间选择", "选择双击按键响应间隔时间", DOUBLE_KEY_TIME, BIT, 4, 5, 8, ("200ms", 2), ("300ms", 3), ("400ms", 4), ("500ms", 5), ("600ms", 6), ("700ms", 7), ("800ms", 8), ("900ms", 9));

config(LEVEL, 0x03);
config(SUB, "LED灯配置", "选择及配置LED状态指示灯");
config(CHECK, "系统指示灯(蓝灯)", "是否使用系统状态指示灯", BLED_DISP_EN, 0);
config(LISTVAL, "蓝灯IO选择", "蓝灯的GPIO口选择", BLED_IO_SEL, BIT, 8, 0xA0, 26, ("PA0", 0xA0), ("PA1", 0xA1), ("PA2", 0xA2), ("PA3", 0xA3), ("PA4", 0xA4), ("PA5", 0xA5), ("PA6", 0xA6), ("PA7", 0xA7), ("PA8", 0xA8), ("PA9", 0xA9), ("PA10", 0xAA),  ("PA11", 0xAB), ("PA12", 0xAC), ("PA13", 0xAD), ("PA14", 0xAE), ("PA15", 0xAF), ("PB0", 0xB0), ("PB1", 0xB1), ("PB2", 0xB2), ("PB3", 0xB3), ("PB4", 0xB4), ("PB5", 0xB5), ("PB6", 0xB6), ("PB7", 0xB7), ("PB8", 0xB8), ("PB9", 0xB9), BLED_DISP_EN);
config(CHECK, "充电指示灯(红灯)", "是否使用充电状态指示灯", RLED_DISP_EN, 0);
config(LISTVAL, "红灯IO选择", "红灯的GPIO口选择", RLED_IO_SEL, BIT, 8, 0xA0, 26, ("PA0", 0xA0), ("PA1", 0xA1), ("PA2", 0xA2), ("PA3", 0xA3), ("PA4", 0xA4), ("PA5", 0xA5), ("PA6", 0xA6), ("PA7", 0xA7), ("PA8", 0xA8), ("PA9", 0xA9), ("PA10", 0xAA),  ("PA11", 0xAB), ("PA12", 0xAC), ("PA13", 0xAD), ("PA14", 0xAE), ("PA15", 0xAF), ("PB0", 0xB0), ("PB1", 0xB1), ("PB2", 0xB2), ("PB3", 0xB3), ("PB4", 0xB4), ("PB5", 0xB5), ("PB6", 0xB6), ("PB7", 0xB7), ("PB8", 0xB8), ("PB9", 0xB9), RLED_DISP_EN);

config(CHECK, "开机状态配置LED", "是否配置开机LED指示灯状态", LED_PWRON_CONFIG_EN, 0, BLED_DISP_EN);
config(LED, "开机", "配置开机状态下的闪灯状态", LED_POWERON, 0x00, 0x01, 10, 255, 		LED_PWRON_CONFIG_EN);

config(CHECK, "关机状态配置LED", "是否配置等待关机状态指示灯", LED_PWROFF_CONFIG_EN, 0, BLED_DISP_EN);
config(LED, "关机", "配置关机状态下的闪灯状态", LED_POWEROFF, 0x00, 0x15, 4, 255, 		LED_PWROFF_CONFIG_EN);

config(CHECK, "蓝牙配对状态配置LED", "是否配置蓝牙配对状态指示灯(蓝牙未连接)", LED_BTPAIR_CONFIG_EN, 0, BLED_DISP_EN);
config(LED, "配对", "配置配对状态下的闪灯状态", LED_PAIRING, 0x00, 0x03, 3, 6, 			LED_BTPAIR_CONFIG_EN);

config(CHECK, "蓝牙已连接状态配置LED", "是否配置蓝牙已连接状态指示灯", LED_BTCONN_CONFIG_EN, 0, BLED_DISP_EN);
config(LED, "已连接", "配置已连接状态下的闪灯状态", LED_CONNECTED, 0x00, 0x02, 2, 86, 	LED_BTCONN_CONFIG_EN);


config(LEVEL, 0x0E);
config(SUB, "产测保留", "产测时保留的一些参数");
config(KEEP);
config(BYTE, "产测OSC基础电容", "同时配置24晶振OSCI与OSCO负载电容,0: 0PF, 1:6PF", OSC_BOTH_CAP, 0, 1, 0);
config(BYTE, "产测OSCI电容", "独立配置24M晶振OSCI负载电容，单位0.25PF。OSCI电容大小：n * 0.25PF + 基础电容, 不要改动默认值0", OSCI_CAP, 0, 63, 0);
config(BYTE, "产测OSCO电容", "独立配置24M晶振OSCO负载电容，单位0.25PF。OSCO电容大小：n * 0.25PF + 基础电容, 不要改动默认值0", OSCO_CAP, 0, 63, 0);
#config(BUF, "加密狗授权密钥", "程序运行需要判断的合法授权密钥", dongle_soft_key, 20);


config(UPD, 0x1F, 0x0);

config(LEVEL, 0x00);
makecfgfile(xcfg.bin);
makecfgdef(xcfg.h);
xcopy(xcfg.h, ../../xcfg.h);
