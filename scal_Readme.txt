如何執行有scale out/in功能的LBA:
主程式: server_loLB_scal.c
編譯: gcc server_loLB_scal.c  -o <任意filename>
執行方式: ./<任意filename> <local controller要連過來的port>
程式放置位子: 這支程式放在global controller上

副程式:GA_connection_scal.c
編譯: gcc GA_connection_scal.c  -o GA_connection_scal  //名稱請固定，主程式要以此檔名執行該副程式
執行方式: 不用執行，編譯成功後放著，主程式啟動GA時使用
程式放置位子: 這支程式也放在global controller上

副程式: send_scal_in_signal_client.c
編譯: gcc send_scal_in_signal_client.c  -o send_scal_in_signal_client //名稱也請固定，主程式要以此檔名執行該副程式
執行方式: 不用執行，編譯成功後放著，主程式啟動GA時使用
程式放置位子: 這支程式也放在global controller上

副程式: send_scal_out_signal_client.c
編譯: gcc send_scal_out_signal_client.c  -o send_scal_out_signal_client //名稱也請固定，主程式要以此檔名執行該副程式
執行方式: 不用執行，編譯成功後放著，主程式啟動GA時使用
程式放置位子: 這支程式也放在global controller上

輔助程式: scaler_monitor_listener.c
編譯: gcc scaler_monitor_listener.c  -o <任意filename>
執行方式: 在要被scale in/out的controller上跑，這支程式會聽LBA送來的scale in out 訊息，執行的時候要+sudo
程式放置位子: 要被scale in/out的 local controller上

輔助程式: start.sh
程式放置位子: 要被scale in/out的 local controller上
使用方式: scaler_monitor_listener會call他，啟動這一台local controller的monitor
使用前請先打開來確認裡面的監控程式啟動設定，包括監控程式的執行檔名稱，LBA的IP，controller編號，這支程式執行的時候會產生設定檔 pid_of_cli，是給 end.sh執行的時候用的

輔助程式: end.sh
程式放置位子: 要被scale in/out的 local controller上
使用方式: scaler_monitor_listener會call他，關閉這一台local controller的monitor

Config檔:scal_out_ctrl_ip.txt
用途: 給send_scal_in_signal_client跟send_scal_out_signal_client吃的config檔，使他們知道scale local controller的IP
程式放置位子:  global controller

Config檔:scal_out_monitor_port.txt
用途: 跟scaler_monitor_listener溝通的port
程式放置位子: global controller跟要被scale in/out的 local controller上都要放
