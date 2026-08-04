#ifndef _USB_API_H
#define _USB_API_H

u8 usbchk_connect(void);
void pc_remove(void);
void usb_device_process(void);
void usb_connected_sync_volume(void);
void usb_device_enter(u8 enum_type);
void usb_device_exit(void);

#endif // _USB_API_H
