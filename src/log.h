// in order to compile/link, requires:
// 1. line in spec file: "BuildRequires:  pkgconfig(dlog)"
// 2. in cmake file: pkg_check_modules(PKGS REQUIRED ... dlog)

#ifndef __LOG_H__
#define __LOG_H__

#ifndef NO_TIZEN
#include <dlog.h>
#else
#include <stdio.h>
#define LOGE(fmt, args...) printf(fmt, ##args)
#define LOGD(fmt, args...) printf(fmt, ##args)
#define LOGI(fmt, args...) printf(fmt, ##args)
#endif

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "JuvoGL"
#ifndef _ERR
#define _ERR(fmt, args...) LOGE(fmt "\n", ##args)
#endif

#ifndef _DBG
#define _DBG(fmt, args...) LOGD(fmt "\n", ##args)
#endif

#ifndef _INFO
#define _INFO(fmt, args...) LOGI(fmt "\n", ##args)
#endif

#endif /* __LOG_H__ */
