�p����榳scale out/in�\�઺LBA:
�D�{��: server_loLB_scal.c
�sĶ: gcc server_loLB_scal.c  -o <���Nfilename>
����覡: ./<���Nfilename> <local controller�n�s�L�Ӫ�port>
�{����m��l: �o��{����bglobal controller�W

�Ƶ{��:GA_connection_scal.c
�sĶ: gcc GA_connection_scal.c  -o GA_connection_scal  //�W�ٽЩT�w�A�D�{���n�H���ɦW����ӰƵ{��
����覡: ���ΰ���A�sĶ���\���ۡA�D�{���Ұ�GA�ɨϥ�
�{����m��l: �o��{���]��bglobal controller�W

�Ƶ{��: send_scal_in_signal_client.c
�sĶ: gcc send_scal_in_signal_client.c  -o send_scal_in_signal_client //�W�٤]�ЩT�w�A�D�{���n�H���ɦW����ӰƵ{��
����覡: ���ΰ���A�sĶ���\���ۡA�D�{���Ұ�GA�ɨϥ�
�{����m��l: �o��{���]��bglobal controller�W

�Ƶ{��: send_scal_out_signal_client.c
�sĶ: gcc send_scal_out_signal_client.c  -o send_scal_out_signal_client //�W�٤]�ЩT�w�A�D�{���n�H���ɦW����ӰƵ{��
����覡: ���ΰ���A�sĶ���\���ۡA�D�{���Ұ�GA�ɨϥ�
�{����m��l: �o��{���]��bglobal controller�W

���U�{��: scaler_monitor_listener.c
�sĶ: gcc scaler_monitor_listener.c  -o <���Nfilename>
����覡: �b�n�Qscale in/out��controller�W�]�A�o��{���|ťLBA�e�Ӫ�scale in out �T���A���檺�ɭԭn+sudo
�{����m��l: �n�Qscale in/out�� local controller�W

���U�{��: start.sh
�{����m��l: �n�Qscale in/out�� local controller�W
�ϥΤ覡: scaler_monitor_listener�|call�L�A�Ұʳo�@�xlocal controller��monitor
�ϥΫe�Х����}�ӽT�{�̭����ʱ��{���Ұʳ]�w�A�]�A�ʱ��{���������ɦW�١ALBA��IP�Acontroller�s���A�o��{�����檺�ɭԷ|���ͳ]�w�� pid_of_cli�A�O�� end.sh���檺�ɭԥΪ�

���U�{��: end.sh
�{����m��l: �n�Qscale in/out�� local controller�W
�ϥΤ覡: scaler_monitor_listener�|call�L�A�����o�@�xlocal controller��monitor

Config��:scal_out_ctrl_ip.txt
�γ~: ��send_scal_in_signal_client��send_scal_out_signal_client�Y��config�ɡA�ϥL�̪��Dscale local controller��IP
�{����m��l:  global controller

Config��:scal_out_monitor_port.txt
�γ~: ��scaler_monitor_listener���q��port
�{����m��l: global controller��n�Qscale in/out�� local controller�W���n��
