ifndef CONTIKI
  $(warning CONTIKI not defined! build may fail!)
  CONTIKI = ./contiki
endif

DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = node_main
HAVE_BANKING=1

#Bank7用于保存设置信息，不用于程序
HIGH_FLASH_BANK=6

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
