prefix = /usr
datadir = $(prefix)/share
iconsdir = $(datadir)/icons
iconstheme = hicolor
iconsbasedir = $(datadir)/icons/$(iconstheme)
icons16dir = $(iconsbasedir)/16x16/apps
icons32dir = $(iconsbasedir)/32x32/apps

kde_prefix = $(prefix)
kde_applicationsdir = $(kde_datadir)/applications/kde4
kde_datadir = $(kde_prefix)/share

TOP_DIR = .
BUILD_DIR = $(TOP_DIR)/build
SRC_DIR = $(TOP_DIR)/src

# Get debug option comming from "configure" : WITH_DEBUG = ["yes" | "no"]
include $(BUILD_DIR)/config.mk

all:
	make -C $(BUILD_DIR) all

$(BUILD_DIR)/src/kscope:
	make -C $(BUILD_DIR) all

install:	$(BUILD_DIR)/src/kscope
	mkdir -p $(DESTDIR)

ifeq ($(WITH_DEBUG), no)
	make -C $(BUILD_DIR) DESTDIR=$(DESTDIR) install/strip
else
	make -C $(BUILD_DIR) DESTDIR=$(DESTDIR) install
endif

	install -D -m 644 $(DESTDIR)$(icons32dir)/kscope.png $(DESTDIR)$(iconsdir)/large/kscope.png
	install -D -m 644 $(DESTDIR)$(icons32dir)/kscope.png $(DESTDIR)$(iconsdir)/kscope.png
	install -D -m 644 $(DESTDIR)$(icons16dir)/kscope.png $(DESTDIR)$(iconsdir)/mini/kscope.png
	mkdir -p $(DESTDIR)$(kde_applicationsdir)
	desktop-file-install --vendor= --dir $(DESTDIR)$(kde_applicationsdir) --add-category=TextEditor $(DESTDIR)$(kde_applicationsdir)/kscope.desktop
