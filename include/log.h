// Requires:
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
#define LOG_TAG "JuvoPlayer"
#ifndef _ERR
#ifndef NO_TIZEN
#define _ERR(fmt, args...) dlog_print(DLOG_ERROR, LOG_TAG, fmt, ##args)
#else
#define _ERR(fmt, args...) LOGE(fmt "\n", ##args)
#endif // #ifndef NO_TIZEN
#endif // #ifndef _ERR

#ifndef _DBG
#ifndef NO_TIZEN
#define _DBG(fmt, args...) dlog_print(DLOG_DEBUG, LOG_TAG, fmt, ##args)
#else
#define _DBG(fmt, args...) LOGD(fmt "\n", ##args)
#endif // #ifndef NO_TIZEN
#endif // #ifndef _DBG

#ifndef _INFO
#ifndef NO_TIZEN
#define _INFO(fmt, args...) dlog_print(DLOG_INFO, LOG_TAG, fmt, ##args)
#else
#define _INFO(fmt, args...) LOGI(fmt "\n", ##args)
#endif // #ifndef NO_TIZEN
#endif // #ifndef _INFO

#endif /* __LOG_H__ */
