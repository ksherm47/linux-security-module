obj-m := kenlex.o 
kenlex-y := kenlex_monitor.o
kenlex_monitor-y := $(LINUX_SOURCE_PATH)/fs/notify $(LINUX_SOURCE_PATH)/fs/notify/inotify
# kenlex_monitor-y := $(LINUX_SOURCE_PATH)/fs/notify/fsnotify.o \
#                     $(LINUX_SOURCE_PATH)/fs/notify/notification.o \
# 					$(LINUX_SOURCE_PATH)/fs/notify/group.o \
# 					$(LINUX_SOURCE_PATH)/fs/notify/mark.o \
# 					$(LINUX_SOURCE_PATH)/fs/notify/fdinfo.o \
# 					$(LINUX_SOURCE_PATH)/fs/notify/inotify/inotify_fsnotify.o \
# 					$(LINUX_SOURCE_PATH)/fs/notify/inotify/inotify_user.o