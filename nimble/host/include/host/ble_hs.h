/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef H_BLE_HS_
#define H_BLE_HS_

/** 
 * 蓝牙Host
 * @brief Bluetooth Host
 * @defgroup bt_host Bluetooth Host
 * @{
 */

#include <inttypes.h>
#include "nimble/hci_common.h"
#include "host/ble_att.h"
#include "host/ble_eddystone.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_hs_adv.h"
#include "host/ble_hs_id.h"
#include "host/ble_hs_hci.h"
#include "host/ble_hs_log.h"
#include "host/ble_hs_mbuf.h"
#include "host/ble_hs_stop.h"
#include "host/ble_ibeacon.h"
#include "host/ble_l2cap.h"
#include "host/ble_sm.h"
#include "host/ble_store.h"
#include "host/ble_uuid.h"
#include "nimble/nimble_npl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_HS_FOREVER              INT32_MAX

/* 连接句柄不存在 */
/** Connection handle not present */
#define BLE_HS_CONN_HANDLE_NONE     0xffff

/**
 * @brief Bluetooth Host Error Code
 * @defgroup bt_host_err Bluetooth Host Error Code
 *
 * Defines error codes returned by Bluetooth host. If error comes from specific
 * component (eg L2CAP or Security Manager) it is shifted by base allowing to
 * identify component.
 * @{
 */
/*	蓝牙主机错误代码
	定义蓝牙主机返回的错误代码,如果错误来自特定组件(例如 L2CAP 或安全管理器)
	则它会通过基数进行转移,从而允许识别组件 */
#define BLE_HS_EAGAIN               1
#define BLE_HS_EALREADY             2
#define BLE_HS_EINVAL               3
#define BLE_HS_EMSGSIZE             4
#define BLE_HS_ENOENT               5
#define BLE_HS_ENOMEM               6
#define BLE_HS_ENOTCONN             7
#define BLE_HS_ENOTSUP              8
#define BLE_HS_EAPP                 9
#define BLE_HS_EBADDATA             10
#define BLE_HS_EOS                  11
#define BLE_HS_ECONTROLLER          12
#define BLE_HS_ETIMEOUT             13
#define BLE_HS_EDONE                14
#define BLE_HS_EBUSY                15
#define BLE_HS_EREJECT              16
#define BLE_HS_EUNKNOWN             17
#define BLE_HS_EROLE                18
#define BLE_HS_ETIMEOUT_HCI         19
#define BLE_HS_ENOMEM_EVT           20
#define BLE_HS_ENOADDR              21
#define BLE_HS_ENOTSYNCED           22
#define BLE_HS_EAUTHEN              23
#define BLE_HS_EAUTHOR              24
#define BLE_HS_EENCRYPT             25
#define BLE_HS_EENCRYPT_KEY_SZ      26
#define BLE_HS_ESTORE_CAP           27
#define BLE_HS_ESTORE_FAIL          28
#define BLE_HS_EPREEMPTED           29
#define BLE_HS_EDISABLED            30
#define BLE_HS_ESTALLED             31

/** Error base for ATT errors */
/* ATT错误基址 */
#define BLE_HS_ERR_ATT_BASE         0x100

/** Converts error to ATT base */
/* 转换错误到ATT基址 */
#define BLE_HS_ATT_ERR(x)           ((x) ? BLE_HS_ERR_ATT_BASE + (x) : 0)

/** Error base for HCI errors */
/* HCI错误的错误基址 */
#define BLE_HS_ERR_HCI_BASE         0x200

/** Converts error to HCI base */
/* 转换错误到HCI基址 */
#define BLE_HS_HCI_ERR(x)           ((x) ? BLE_HS_ERR_HCI_BASE + (x) : 0)

/** Error base for L2CAP errors */
/* L2CAP错误基址 */
#define BLE_HS_ERR_L2C_BASE         0x300

/** Converts error to L2CAP base */
/* 转换错误到L2CAP基址 */
#define BLE_HS_L2C_ERR(x)           ((x) ? BLE_HS_ERR_L2C_BASE + (x) : 0)

/** Error base for local Security Manager errors */
/* 本地安全管理器错误基址 */
#define BLE_HS_ERR_SM_US_BASE       0x400

/** Converts error to local Security Manager base */
/* 转换到   本地安全管理器的错误基址 */
#define BLE_HS_SM_US_ERR(x)         ((x) ? BLE_HS_ERR_SM_US_BASE + (x) : 0)

/** Error base for remote (peer) Security Manager errors */
/* 远程(对等)的安全管理器的错误基址 */
#define BLE_HS_ERR_SM_PEER_BASE     0x500

/** Converts error to remote (peer) Security Manager base */
/* 转换到远程(对等)的安全管理器的错误基址 */
#define BLE_HS_SM_PEER_ERR(x)       ((x) ? BLE_HS_ERR_SM_PEER_BASE + (x) : 0)

/** Error base for hardware errors */
/* 硬件错误基址 */
#define BLE_HS_ERR_HW_BASE          0x600

/** Converts error to hardware error  base */
/* 转换到硬件的错误基址 */
#define BLE_HS_HW_ERR(x)            (BLE_HS_ERR_HW_BASE + (x))

/**
 * @}
 */

/**
 * @brief Bluetooth Host Configuration
 * @defgroup bt_host_conf Bluetooth Host Configuration
 *
 * @{
 */

/**
 * 设备的本地输入输出能力
 * @brief Local Input-Output capabilities of device
 * @defgroup bt_host_io_local Local Input-Output capabilities of device
 *
 * @{
 */

/** DisplayOnly IO capability */
/* 仅显示IO功能 */
#define BLE_HS_IO_DISPLAY_ONLY              0x00

/** DisplayYesNo IO capability */
/* 显示YesNO   IO 能力 */
#define BLE_HS_IO_DISPLAY_YESNO             0x01

/** KeyboardOnly IO capability */
/* 仅键盘IO更能 */
#define BLE_HS_IO_KEYBOARD_ONLY             0x02

/** NoInputNoOutput IO capability */
/* 无输入无输出 IO 能力 */
#define BLE_HS_IO_NO_INPUT_OUTPUT           0x03

/** KeyboardDisplay Only IO capability */   
/* 仅键盘显示 IO 功能 */
#define BLE_HS_IO_KEYBOARD_DISPLAY          0x04

/**
 * @}
 */

/**
 * LE 密钥分发
 * @brief LE key distribution
 * @defgroup bt_host_key_dist LE key distribution
 *
 * @{
 */

/** Distibute LTK */
/* 分发LTK */
#define BLE_HS_KEY_DIST_ENC_KEY              0x01

/** Distribute IRK */
/* 分发IRK */
#define BLE_HS_KEY_DIST_ID_KEY               0x02

/** CSRK distibution and LinkKey are not supported */
/* 不支持CSRK分发和LinKKey */

/**
 * @}
 */

/** @brief Stack reset callback
 *
 * @param reason Reason code for reset
 */
/* 栈重置回调 */
typedef void ble_hs_reset_fn(int reason);


/** @brief Stack sync callback */
/* 栈同步回调 */
typedef void ble_hs_sync_fn(void);

/** @brief Bluetooth Host main configuration structure
 *
 * Those can be used by application to configure stack.
 *
 * The only reason Security Manager (sm_ members) is configurable at runtime is
 * to simplify security testing. Defaults for those are configured by selecting
 * proper options in application's syscfg.
 */
 
 /* 蓝牙Host主要配置结构
 	应用程序可以使用它们来配置堆栈。
 	安全管理器(sm_members)可在运行时配置的唯一原因是为了简化安全测试。
 	这些默认值是通过在应用程序的syscfg.h中选择适当的选项来配置的 */
struct ble_hs_cfg {
    /**
     * An optional callback that gets executed upon registration of each GATT
     * resource (service, characteristic, or descriptor).
     */
    /* 注册每个 GATT 资源（服务、特征或描述符）时执行的可选回调 */
    ble_gatt_register_fn *gatts_register_cb;

    /**
     * An optional argument that gets passed to the GATT registration
     * callback.
     */
    /* 传递给GATT回调函数的参数 */
    void *gatts_register_arg;

    /** Security Manager Local Input Output Capabilities */
	/* 安全管理器本地输入输出功能 */
    uint8_t sm_io_cap;

    /** @brief Security Manager OOB flag
     *
     * If set proper flag in Pairing Request/Response will be set.
     */
    /* 安全管理器 OOB 标志
       如果在配对请求/响应中设置合适的标志，这个将会被设置 */
    unsigned sm_oob_data_flag:1;

    /** @brief Security Manager Bond flag
     *
     * If set proper flag in Pairing Request/Response will be set. This results
     * in storing keys distributed during bonding.
     */
    /* 安全管理器Bond标志
	   如果在配对请求/响应中设置合适的标志,这个将会被设置
	   这将导致在绑定期间会存储分发密钥 */
    unsigned sm_bonding:1;

    /** @brief Security Manager MITM flag
     *
     * If set proper flag in Pairing Request/Response will be set. This results
     * in requiring Man-In-The-Middle protection when pairing.
     */
    /* 安全管理器 MITM 标志
	   如果在配对请求/响应中设置合适的标志,这个将会被设置
	   这个请求在配对时需要中间人保护 */
    unsigned sm_mitm:1;

    /** @brief Security Manager Secure Connections flag
     *
     * If set proper flag in Pairing Request/Response will be set. This results
     * in using LE Secure Connections for pairing if also supported by remote
     * device. Fallback to legacy pairing if not supported by remote.
     */
	/* 安全管理器安全连接标志
	   如果在配对请求/响应中设置了合适的标志,则会被设置
	   如果远程设备也支持,这将导致使用 LE 安全连接进行配对
	   如果遥控器不支持,则回退到传统配对
    */
    unsigned sm_sc:1;

    /** @brief Security Manager Key Press Notification flag
     *
     * Currently unsupported and should not be set.
     */
    /* 安全管理器按键通知标志 
	   目前不支持且不应设置 */
    unsigned sm_keypress:1;

    /** @brief Security Manager Local Key Distribution Mask */
	/* 安全管理器本地密钥分发掩码 */
    uint8_t sm_our_key_dist;

    /** @brief Security Manager Remote Key Distribution Mask */
	/* 安全管理器远程密钥分发掩码 */
    uint8_t sm_their_key_dist;

    /** @brief Stack reset callback
     *
     * This callback is executed when the host resets itself and the controller
     * due to fatal error.
     */
    /* 堆栈重置回调
	   当主机由于致命错误而重置自身和控制器时，执行此回调 */
    ble_hs_reset_fn *reset_cb;

    /** @brief Stack sync callback
     *
     * This callback is executed when the host and controller become synced.
     * This happens at startup and after a reset.
     */
    /* 堆栈同步回调
       当主机和控制器同步时执行此回调。这发生在启动时和重置后 */
    ble_hs_sync_fn *sync_cb;

    /** Callback to handle generation of security keys */
	/* 处理生成安全密钥的回调 */
    ble_store_gen_key_fn *store_gen_key_cb;

    /* XXX: These need to go away. Instead, the nimble host package should
     * require the host-store API (not yet implemented)..
     */
    /** Storage Read callback handles read of security material */
	
	/* XXX:这些需要消失，相反，nimble的主机包应该需要host-store API(还没实现)  
	   读安全材料的存储读回调函数 */
    ble_store_read_fn *store_read_cb;

    /** Storage Write callback handles write of security material */
	/* 写安全材料的存储写回调函数 */
    ble_store_write_fn *store_write_cb;

    /** Storage Delete callback handles deletion of security material */
	/* 删除安全材料的存储删除回调函数 */
    ble_store_delete_fn *store_delete_cb;

    /** @brief Storage Status callback.
     *
     * This callback gets executed when a persistence operation cannot be
     * performed or a persistence failure is imminent. For example, if is
     * insufficient storage capacity for a record to be persisted, this
     * function gets called to give the application the opportunity to make
     * room.
     */
    /* 存储状态回调 
	   当持久化操作无法执行时执行此回调
	   已执行或持久性失败即将发生,例如,如果存储容量不足以持久保存一条记录,
	   则此调用函数以使应用程序有机会腾出空间。*/
    ble_store_status_fn *store_status_cb;

    /** An optional argument that gets passed to the storage status callback. */
	/* 传递给存储状态回调函数的参数 */
    void *store_status_arg;
};

extern struct ble_hs_cfg ble_hs_cfg;

/**
 * @}
 */

/**
 * @brief Indicates whether the host is enabled.  The host is enabled if it is
 * starting or fully started.  It is disabled if it is stopping or stopped.
 *
 * @return 1 if the host is enabled;
 *         0 if the host is disabled.
 */
/** 表示host是否启用,如果host已启用,则开始或完全开始,如果正在停止或已停止,则该功能被禁用
  * @return 1 如果host已启用
  * 		0 如果host被禁用 */
int ble_hs_is_enabled(void);

/**
 * Indicates whether the host has synchronized with the controller.
 * Synchronization must occur before any host procedures can be performed.
 *
 * @return 1 if the host and controller are in sync;
 *         0 if the host and controller are out of sync.
 */
/** 指示host是否已与控制器同步,必须在执行任何host程序之前进行同步。
  * @return 1 如果主机和控制器同步
  * 		0 如果主机和控制器不同步 */
int ble_hs_synced(void);

/**
 * Synchronizes the host with the controller by sending a sequence of HCI
 * commands.  This function must be called before any other host functionality
 * is used, but it must be called after both the host and controller are
 * initialized.  Typically, the host-parent-task calls this function at the top
 * of its task routine.  This function must only be called in the host parent
 * task.  A safe alternative for starting the stack from any task is to call
 * `ble_hs_sched_start()`.
 *
 * If the host fails to synchronize with the controller (if the controller is
 * not fully booted, for example), the host will attempt to resynchronize every
 * 100 ms. For this reason, an error return code is not necessarily fatal.
 *
 * @return 0 on success; nonzero on error.
 */
/** 通过发送一系列HCI命令来同步Host与控制器,该函数必须在使用任何其他Host功能之前调用,
  *	但必须在Host和控制器初始化之后调用,通常,Host父任务在其任务例程的顶部调用此函数,
  *	该函数只能在宿主父任务中调用,从任何任务启动堆栈的一个安全替代方法是调用“ble_hs_sched_start()”
  *	如果Host无法与控制器同步(例如,如果控制器未完全启动),Host将尝试每100毫秒重新同步一次 
  *	因此,错误返回代码不一定是致命的。
  * @成功则返回0； 错误时非零 */
int ble_hs_start(void);

/**
 * Enqueues a host start event to the default event queue.  The actual host
 * startup is performed in the host parent task, but using the default queue
 * here ensures the event won't run until the end of main() when this is
 * called during system initialization.  This allows the application to
 * configure the host package in the meantime.
 *
 * If auto-start is disabled, the application should use this function to start
 * the BLE stack.  This function can be called at any time as long as the host
 * is stopped.  When the host successfully starts, the application is notified
 * via the ble_hs_cfg.sync_cb callback.
 */
/* 将Host启动事件排入默认事件队列,实际的Host启动是在主机父任务中执行的,
   但这里使用默认队列可确保事件不会运行,直到系统初始化期间调用main()结束时。 
   这允许应用程序同时配置Host包,如果自动启动被禁用,应用程序应使用此函数来启动BLE堆栈,
   只要Host停止,就可以随时调用此函数,当Host成功启动时,应用程序会通过ble_hs_cfg.sync_cb回调函数收到通知 */
void ble_hs_sched_start(void);

/**
 * Causes the host to reset the NimBLE stack as soon as possible.  The
 * application is notified when the reset occurs via the host reset callback.
 *
 * @param reason The host error code that gets passed to the reset callback.
 */
/* 让Host尽快重置NimBLE堆栈,当发生重置时,通过主机重置回调通知应用程序。
   @param Reason 传递给重置回调的主机错误代码 */
void ble_hs_sched_reset(int reason);

/**
 * Designates the specified event queue for NimBLE host work. By default, the
 * host uses the default event queue and runs in the main task. This function
 * is useful if you want the host to run in a different task.
 *
 * @param evq The event queue to use for host work.
 */
/* 指定Host工作的指定事件队列,默认情况下,主机使用默认事件队列并在主任务中运,如果您希望主机在不同的任务中运行,则此功能非常有用
   @param evq 用于主机工作的事件队列 */
void ble_hs_evq_set(struct ble_npl_eventq *evq);

/**
 * Initializes the NimBLE host. This function must be called before the OS is
 * started. The NimBLE stack requires an application task to function.  One
 * application task in particular is designated as the "host parent task". In
 * addition to application-specific work, the host parent task does work for
 * NimBLE by processing events generated by the host.
 */
/* 初始化NimBLE主机,该函数必须在操作系统启动之前调用,NimBLE 堆栈需要应用程序任务才能运行
   一个应用程序任务被特别指定为“Host父任务”,除了特定于应用程序的工作之外,Host父任务还通过处理主机生成的事件来为NimBLE工作。 */
void ble_hs_init(void);

/**
 * @brief Called when the system is shutting down.  Stops the BLE host.
 *
 * @param reason                The reason for the shutdown.  One of the
 *                                  HAL_RESET_[...] codes or an
 *                                  implementation-defined value.
 *
 * @return                      SYSDOWN_IN_PROGRESS. 
 */
/*	在系统关闭时调用,停止 BLE 主机。
	关闭的原因,HAL_RESET_[...]代码或实现定义的值,的其中的一个 */
int ble_hs_shutdown(int reason);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
