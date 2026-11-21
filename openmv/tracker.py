# OpenMV v5 固件（csi 模块）— 绿色色块追踪 + 舵机云台串口指令
# 串口帧格式：<水平命令><垂直命令>'\n'，9600 波特率，8N1

import csi
import time
import image
from machine import UART

# 绿色阈值：上线前务必用 OpenMV 阈值编辑器现场重新调（不同光线差别很大）
GREEN_THRESHOLD = (30, 80, -60, 5, 20, 28)

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)      # QVGA 比 VGA 快很多，追踪够用
csi0.snapshot(time=2000)      # 预热约 2 秒，让自动曝光/增益/白平衡收敛（别删！）
csi0.auto_gain(False)         # v5 命名：auto_gain / auto_whitebal / auto_exposure
csi0.auto_whitebal(False)
csi0.auto_exposure(False)     # 锁曝光，绿色阈值才稳定

uart = UART(3, 9600)          # 必须和下位机一致；H7 上 UART3 = P4(TX)/P5(RX)

clock = time.clock()

# 以下阈值是 QVGA(320x240) 下的值；若改回 VGA(640x480)，全部乘 2
DEAD_X = 20                   # 水平死区：偏差小于它就不动
DEAD_Y = 15                   # 垂直死区
FAST_X = 35                   # 水平偏差超过它用快速档
FAST_Y = 30                   # 垂直偏差超过它用快速档

DEBUG_ONLY_DELTA = False      # 调试阶段：True = 只打印偏差、不发串口；验证完改成 False

def find_max(blobs):
    max_blob = None
    max_size = 0
    for b in blobs:
        if b.pixels > max_size:
            max_size = b.pixels
            max_blob = b
    return max_blob

def send_cmd(h, v):
    # 一帧 3 字节：水平 + 垂直 + '\n'（帧尾），下位机读到 '\n' 才处理
    uart.write(bytes([ord(h), ord(v), 10]))

frame_cnt = 0

while True:
    clock.tick()
    # 每帧旋转 90°：ROTATE_270 = HMIRROR|TRANSPOSE（逆时针）；若方向反了就换 ROTATE_90
    img = csi0.snapshot().scale(hint=image.ROTATE_270)

    img_center_x = img.width() // 2
    img_center_y = img.height() // 2

    # 过滤小噪点，merge 合并同色碎块
    blobs = img.find_blobs(
        [GREEN_THRESHOLD],
        pixels_threshold=100,     # 目标远/小时可调小到 50
        area_threshold=100,
        merge=True,
    )

    send1 = 'o'                   # 'o' = 该轴不动（先默认居中，避免漏赋值）
    send2 = 'o'
    delta_x = 0                   # 没找到目标时保持 0
    delta_y = 0

    if blobs:
        b = find_max(blobs)
        delta_x = b.cx - img_center_x
        delta_y = b.cy - img_center_y

        # ---- 水平 ----
        if abs(delta_x) <= DEAD_X:
            send1 = 'o'
        elif delta_x > FAST_X:
            send1 = 'a'           # 目标偏右且偏差大 → 快速右转
        elif delta_x > 0:
            send1 = 'b'           # 偏右小偏差 → 慢速右转
        elif delta_x < -FAST_X:
            send1 = 'c'           # 偏左且偏差大 → 快速左转
        else:
            send1 = 'd'           # 偏左小偏差 → 慢速左转

        # ---- 垂直（图像 y 向下为正：dy>0 表示目标在画面下方）----
        if abs(delta_y) <= DEAD_Y:
            send2 = 'o'
        elif delta_y > FAST_Y:
            send2 = 'x'           # 偏下大偏差 → 快速下俯
        elif delta_y > 0:
            send2 = 'z'           # 偏下小偏差 → 慢速下俯
        elif delta_y < -FAST_Y:
            send2 = 'y'           # 偏上大偏差 → 快速上仰
        else:
            send2 = 'w'           # 偏上小偏差 → 慢速上仰

        # 调试显示：红框 + 中心十字
        img.draw_rectangle(b.rect, color=(255, 0, 0))
        img.draw_cross((b.cx, b.cy), color=(255, 0, 0))

    # 调试阶段只算偏差，不发送；DEBUG_ONLY_DELTA 改 False 后恢复每帧发送
    if not DEBUG_ONLY_DELTA:
        send_cmd(send1, send2)

    frame_cnt += 1
    if frame_cnt % 10 == 0:
        print('delta:', delta_x, delta_y,
              'center:', (img_center_x, img_center_y),
              'blob:', (b.cx, b.cy) if blobs else None)
    if frame_cnt % 30 == 0:
        print('fps:', clock.fps(), 'send:', send1, send2)
