UTILITIES = \
	APLAY \
	BASENAME \
	CAT \
	CLEAR \
	CP \
	CRC32 \
	DIRNAME \
	DISPLAYCTL \
	DSTART \
	ECHO \
	ENV \
	FALSE \
	GREP \
	HEAD \
	HEXDUMP \
	INIT \
	JSON \
	KEYBOARDCTL \
	KILL \
	LINK  \
	LS \
	MKDIR \
	MV \
	NETCTL\
	NOW \
	OPEN \
	PANIC \
	PIANO \
	PLAY \
	POWERCTL \
	PWD	\
	RMDIR \
	SETTINGSCTL \
	SOUNDCTL \
	SYSFETCH \
	TAC \
	TOUCH \
	TRUE \
	UNAME \
	UNLINK \
	UNZIP \
	UPTIME \
	YES \
	ZIP

APLAY_LIBS = audio
APLAY_NAME = aplay

BASENAME_LIBS = system io
BASENAME_NAME = basename

CAT_LIBS = system io
CAT_NAME = cat

CLEAR_LIBS = system io
CLEAR_NAME = clear

CP_LIBS = system io
CP_NAME = cp

CRC32_LIBS = system io compression
CRC32_NAME = crc32

PLAY_LIBS = system io
PLAY_NAME = play

DSTART_LIBS = system io
DSTART_NAME = dstart

DIRNAME_LIBS = system io
DIRNAME_NAME = dirname

ECHO_LIBS = system io
ECHO_NAME = echo

ENV_LIBS = system io
ENV_NAME = env

GREP_LIBS = system io
GREP_NAME = grep

HEAD_LIBS = system io
HEAD_NAME = head

HEXDUMP_LIBS = system io
HEXDUMP_NAME = hexdump

INIT_LIBS = system io
INIT_NAME = init

JSON_LIBS = system io
JSON_NAME = json

KILL_LIBS = system io
KILL_NAME = kill

LINK_LIBS = system io
LINK_NAME = link

LS_LIBS = system io
LS_NAME = ls

MKDIR_LIBS = system io
MKDIR_NAME = mkdir

MV_LIBS = system io
MV_NAME = mv

NOW_LIBS = system io
NOW_NAME = now

OPEN_LIBS = system io
OPEN_NAME = open

PANIC_LIBS = system io
PANIC_NAME = panic

RMDIR_LIBS = system io
RMDIR_NAME = rmdir

SYSFETCH_LIBS = system io
SYSFETCH_NAME = sysfetch

TAC_LIBS = system io
TAC_NAME = tac

TOUCH_LIBS = system io
TOUCH_NAME = touch

UNLINK_LIBS = system io
UNLINK_NAME = unlink

UPTIME_LIBS = system io
UPTIME_NAME = uptime

UNAME_LIBS = system io
UNAME_NAME = uname

TRUE_LIBS = system io
TRUE_NAME = true

FALSE_LIBS = system io
FALSE_NAME = false

YES_LIBS = system io
YES_NAME = yes

PWD_LIBS = system io
PWD_NAME = pwd

PIANO_LIBS = system io
PIANO_NAME = piano

DISPLAYCTL_LIBS = system io
DISPLAYCTL_NAME = displayctl

KEYBOARDCTL_LIBS = system io
KEYBOARDCTL_NAME = keyboardctl

NETCTL_LIBS = system io
NETCTL_NAME = netctl

POWERCTL_LIBS = system io
POWERCTL_NAME = powerctl

SETTINGSCTL_LIBS = settings async system io
SETTINGSCTL_NAME = settingsctl

SOUNDCTL_LIBS = 
SOUNDCTL_NAME = soundctl

WALLPAPERCTL_LIBS = graphic system io
WALLPAPERCTL_NAME = wallpaperctl

ZIP_LIBS = file system io compression
ZIP_NAME = zip

UNZIP_LIBS = file system io compression
UNZIP_NAME = unzip

define UTIL_TEMPLATE =

$(1)_BINARY  = $(BUILD_DIRECTORY_UTILITIES)/$($(1)_NAME)
$(1)_SOURCE  = userspace/utilities/$($(1)_NAME).cpp
$(1)_OBJECT  = $$(patsubst userspace/utilities/%.cpp, $$(BUILDROOT)/userspace/utilities/%.o, $$($(1)_SOURCE))

TARGETS += $$($(1)_BINARY)
OBJECTS += $$($(1)_OBJECT)

$$($(1)_BINARY): $$($(1)_OBJECT) $$(patsubst %, $$(BUILD_DIRECTORY_LIBS)/lib%.a, $$($(1)_LIBS) system) $(CRTS)
	$$(DIRECTORY_GUARD)
	@echo [$(1)] [LD] $($(1)_NAME)
	@$(CXX) $(LDFLAGS) -o $$@ $$($(1)_OBJECT) $$(patsubst %, -l%, $$($(1)_LIBS)) -lsystem
	@if $(CONFIG_STRIP); then \
		echo [$(1)] [STRIP] $($(1)_NAME); \
		$(STRIP) $$@; \
	fi

$$($(1)_OBJECT): $$($(1)_SOURCE)
	$$(DIRECTORY_GUARD)
	@echo [$(1)] [CXX] $$<
	@$(CXX) $(CXXFLAGS) -c -o $$@ $$<

endef

$(foreach util, $(UTILITIES), $(eval $(call UTIL_TEMPLATE,$(util))))
