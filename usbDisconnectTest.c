#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include <errno.h>

int main() {
	int fd = openat(-100, "/dev/bus/usb/001/004", O_RDWR, 0);
	struct usbdevfs_ioctl Numba;
	int ifnumber = 0;
	Numba.ifno = ifnumber;
	Numba.ioctl_code = USBDEVFS_DISCONNECT;
	printf("%p\n", &Numba);
	ioctl(fd, USBDEVFS_IOCTL, &Numba);
};
