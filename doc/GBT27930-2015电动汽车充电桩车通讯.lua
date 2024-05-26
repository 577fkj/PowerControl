------------------------------------------------------------------
-- 充电桩
-- 2023-12-07  V1.0  基于坛友 那年秋天的风 分享的J1939开发
------------------------------------------------------------------

--分窗标志定义
WIN_0 = 0x0001
WIN_1 = 0x0002
WIN_2 = 0x0004
WIN_3 = 0x0008
WIN_4 = 0x0010
WIN_5 = 0x0020
WIN_6 = 0x0040
WIN_7 = 0x0080
WIN_8 =  0x0100
WIN_9 =  0x0200
WIN_10 = 0x0400
WIN_11 = 0x0800
WIN_12 = 0x1000
WIN_13 = 0x2000
WIN_14 = 0x4000
WIN_15 = 0x8000


bin = {}

--J1939 解码函数，返回的字符串将展示到数据窗口
--id 帧ID
--std_ext 0表示标准帧, 1表示扩展帧
--data_remote 0表示数据帧, 1表示远程帧
--datalen 数据长度，字节
--data_bin 二进制数据
--函数返回: ret1解码字符串 , ret2分窗控制字
function can_decoder(id, std_ext, data_remote, datalen, data_bin)
	local ret1 = ""		--返回值1 解码显示结果
	local ret2 = 0  	--返回值2 分窗标志
	local ret3 = ""
	local id_ =0
	local id_temp = 0
	local s0 = ""
	local i
	local R1 = 0   --保留位
	local DP = 0   --数据页	
	local PF = 0   --PDU格式
	local PS = 0   --特定PDU
	local SA = 0   --源地址
	local PGN= 0   --PGN
	--逻辑与 and，逻辑或 or，不等于 ~=

	--if std_ext ~= 1 or data_remote ~=1 then--不是扩展帧或者不是数据帧，直接返回
	--	return ret1, ret2, ret3
	--end

	--将字符串转换到整数数组存放
	for i = 1,#data_bin,1 do
		bin[i] = tonumber(string.byte(data_bin, i,i))
	end	

	id_ = tonumber(id)

	--处理优先级
	id_temp = id_ & 0x1C000000
	id_temp = id_temp>>26		
	--ret1 = ret1.."优先级："..tostring(id_temp)
	
	--处理保留位
	id_temp = id_ & 0x02000000
	R1 = id_temp>>25		

	--处理数据页
	id_temp = id_ & 0x01000000
	DP = id_temp>>24	
	--ret1 = ret1.."  数据页："..tostring(DP)

	--处理PF
	id_temp = id_ & 0x00FF0000
	PF = id_temp>>16	

	--处理PS
	id_temp = id_ & 0x0000FF00
	PS = id_temp>>8	

	--处理SA
	id_temp = id_ & 0x000000FF
	SA = id_temp>>0
	--ret1 = ret1..string.format("  源地址: %02d", SA)

	if (SA==86) then
	ret1 = ret1.."充电机->BMS"
   elseif  (SA==244) then
		ret1 = ret1.."BMS->充电机"
    else
		ret1 = ret1.."未知地址"
	end
	
	ret1 = ret1..string.format(" PF= %02x ", PF)



	--if PF < 240 then--如果PF<240 则PS=0
	--	PS = 0
	--end

	--计算PGN 	PGN=R1 | DP | PF | PS
	PGN = R1<<17
	PGN = PGN |  (DP<<16)
	PGN = PGN |  (PF<<8)
	PGN = PGN |  (PS<<0)
	--ret1 = ret1..string.format("  PGN： %5d", PGN)


	s0,ret2,ret3=pgn_response(PF)
	ret1=ret1..s0

	return ret1,ret2,ret3
end


--测试设备=山东烟台龙泵ECU+锡柴发动机
function pgn_response(PF)
	local s1 = ""
	local s2 = 0
	local s3 = ""
	local speed=0


	if PF==0X01 then 
		s1 = "@CRM 充电机辨识报文"
        if(bin[1]==0x00) then
			s1 = s1.."|BMS不能辨识"
		elseif (bin[1]==0xAA) then
			s1 = s1.."|BMS能辨识"
		end

		s1 = s1..string.format("|编号:%d", (bin[5]<<24 | bin[4]<<16 | bin[3]<<8 | bin[2]))

		s1 = s1.. string.format("|编码:0x%08x", (bin[8]<<16 | bin[7]<<8 | bin[6]))
	elseif PF==0X07 then 
		s1 = "@CTS 充电机发送时间同步信息"
	elseif PF==0X08 then 
		s1 = "@CML 充电机最大输出能力"
	elseif PF==0X09 then 
		s1 = "@BRO 电池充电准备就绪状态"
	elseif PF==0X0a then 
		s1 = "@CRO 充电机准备就绪状态"
		if(bin[1]==0x00) then
			s1 = s1.."|未准备就绪"	
	    elseif(bin[1]==0xaa) then 
			s1 = s1.."|准备就绪"	
	    end
	elseif PF==0X10 then 
		s1 = "@BCL 电池充电需求"

    s1 = s1..string.format("|电压:%d", (bin[2]<<8 | bin[1]))
    s1 = s1..string.format("|电流:%d|", (bin[4]<<8 | bin[3]))

    if(bin[5]==1) then
      s1 =s1.. "恒压充电"
    elseif (bin[5]==2) then
      s1 =s1.. "恒流充电"
    end
      
		s2 = s2 | WIN_0

	elseif PF==0X12 then
		s1 = "@CCS 充电机充电状态"
    s1 = s1..string.format("|电压:%d", (bin[2]<<8 | bin[1]))
    s1 = s1..string.format("|电流:%d", (bin[4]<<8 | bin[3]))



	elseif PF==0X13 then
		s1 = "@BSM 动力蓄电池状态信息报文"
    s1 = s1..string.format("|编号:%d", bin[1])
    s1 = s1..string.format("|最高温度:%d", bin[2]-50)
    s1 = s1..string.format("|最高温监测点编号:%d", bin[3])
    s1 = s1..string.format("|最低温度:%d", bin[4]-50)
    s1 = s1..string.format("|最低温监测点编号:%d", bin[5])


	elseif PF==0X19 then
		s1 = "@BST BMS中止充电"
	
	elseif PF==0X1a then
		s1 = "@CST 充电机中止充电"
	elseif PF==0X1C then
		s1 = "@BSD BMS统计数据"
	elseif PF==0X1D then
		s1 = "@CSD 充电机统计数据"
    
    
	elseif PF==0X26 then
		s1 = "@CHM 充电机握手"
    s1 = s1.."|通讯协议版本V"
    s1 = s1..string.format("%d", bin[2])
    s1 = s1.."."
    s1 = s1..string.format("%d", bin[1])
	elseif PF==0X27 then
		s1 = "@BHM BMS握手"  
    s1 = s1.."|最高允许电压"
		s1 = s1..string.format("%d", bin[2]<<8 | bin[1])
	elseif PF==0XEC then
		s1 = "@TP.CM 连接管理"
        if(bin[1]==16) then
			s1 = s1.."|请求发送"
			s1 = s1..string.format("|长度:%d", bin[3]<<8 | bin[2])
			s1 = s1..string.format("|包数:%d", bin[4])

			if(bin[7]==2) then
				s1 = s1.."|BRM BMS和车辆辨识报文"	
        
        
        
		    end		

		elseif (bin[1]==17) then
			s1 = s1.."|准备发送"
			s1 = s1..string.format("|包数:%d", bin[2])
			s1 = s1..string.format("|下一个编号:%d", bin[3])
			
			if(bin[7]==2) then
				s1 = s1.."|BMS和车辆辨识报文"	
		    end		
		elseif (bin[1]==19) then
			s1 = s1.."|结束应答"
		end
	elseif PF==0XEB then
		s1 = "@TP.DT 数据传送"
		s1 = s1..string.format("|序号:%d",bin[1])
	else 
		s1 = "未知报文"
	end




	s3 = string.format("%02x", PF)



	return s1,s2,s3
end




