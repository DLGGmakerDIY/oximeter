# oximeter
脉搏血氧监护仪是一款先进的医疗设备，通过整合MAXREFDES117#-ND传感器（包括MAX1921、MAX14595以及医用级脉搏血氧计MAX30102），并结合M5Stack的core2，实现对病人关键生命体征的实时监测。这款设备专为医疗机构和家庭护理设计，旨在提高病人的健康监护水平，及时发现并预警潜在的健康问题。
## 主要功能：
实时监测：使用高精度光学传感器，持续实时监测病人的血氧饱和度（SpO2）和脉搏数据。
数据分析：利用先进的人工智能算法，分析和处理监测数据，识别病人的健康趋势。
无线传输：通过无线技术将实时数据传输到医生和护理人员的终端设备，确保他们能够随时掌握病人的健康状态。
历史记录：持续记录并存储病人的健康监测数据，生成详细的趋势图表，帮助医护人员进行综合分析。
应用场景：
医院病房：用于住院病人的持续健康监控，提高医生和护士的工作效率，保障病人安全。
家庭护理：适用于慢性病患者或老年人的家庭使用，提供专业的健康监护服务，减少反复奔波医院的频率。
应急医疗：在救护车或其他紧急情况下，快速监测病人生命体征，为急救提供有力的数据支撑。

# BOM
node1:m5stickC+、18650C、GY-MAX30102

node2: SenseCAP Watcher、MAXREFDES117#-ND

gateway：AtomS3LITE

web_display:M5core2Cmd+K 

# 感谢参考项目：
	https://github.com/createskyblue/esp32-web-oximeter 
	https://github.com/moononournation/BloodOxygenHeartRateMeter 
