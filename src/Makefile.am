
lib_LTLIBRARIES=libmozembed_js.la 

AM_CPPFLAGS=-Wall -I. -DXP_UNIX -DOSSP -DEXPORT_JS_API -DJS_HAS_FILE_OBJECT -DHAVE_CONFIG_H

libmozembed_js_la_SOURCES = mozemb_js.cpp
###MozJSDBusMarshalling.cpp
libmozembed_js_la_CFLAGS  = $(MOZEMB_CFLAGS) $(XULU_CFLAGS) $(GTK_CFLAGS) $(JS_CFLAGS) $(DBUS_CFLAGS)
libmozembed_js_la_CXXFLAGS  = $(MOZEMB_CFLAGS) $(XULU_CFLAGS) $(GTK_CFLAGS) $(JS_CFLAGS) $(DBUS_CFLAGS)
libmozembed_js_la_LDFLAGS = $(MOZEMB_LIBS) $(XULU_LIBS) $(GTK_LIBS) $(JS_LIBS) $(DBUS_LIBS) -ldbus-glib-1


