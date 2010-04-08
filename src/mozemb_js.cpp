#include <math.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <map>

#include <assert.h>

#include <jsapi.h>
#include <jsfun.h>


#include <gtk/gtk.h>

//#include "nsStringAPI.h"
#include "gtkmozembed_glue.cpp"

#define check_args(assert, ...) if (!(assert)) { \
		fprintf(stderr, "%s:%d :: ", __FUNCTION__, __LINE__ );\
		fprintf(stderr, __VA_ARGS__);\
		return JS_FALSE; \
	}


using namespace std;

////////////////////////////////////////
/////   MozEmb class
////////////////////////////////////////

typedef struct _MozEmb {
    // my data;
    GtkWidget *window;
    GtkMozEmbed *embed;
    char* name; ///< widget?

} MozEmbData;

/*************************************************************************************************/

static void init_gtk_stuff(MozEmbData* moz) {
    int argc = 0;
    char** argv = {NULL};
    cerr << "init_gtk 1 " << endl;

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

cerr << "init_gtk 1d " << endl;
    GtkWidget* m = gtk_moz_embed_new();

    gtk_widget_set_size_request(m, 600,400);

    gtk_container_add(GTK_CONTAINER(window), m);

cerr << "init_gtk 1a0c " << endl;
    gtk_widget_show(m);
cerr << "init_gtk 1a1c " << endl;
    gtk_widget_show(window);
cerr << "init_gtk 1a2c " << endl;
    cerr << "init_gtk 1 " << endl;

    moz->embed = GTK_MOZ_EMBED(m);
    moz->window = window;
    cerr << "init_gtk 2 " << endl;
}

/** MozEmb Constructor */
static JSBool MozEmbConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    cerr << "*Moz 1 " << endl;
    if (argc == 0) {
        MozEmbData* moz = NULL;

        moz = (MozEmbData*) calloc(sizeof (MozEmbData), 1);

        init_gtk_stuff(moz);

        if (!JS_SetPrivate(cx, obj, moz))
            return JS_FALSE;
    cerr << "*Moz 2 " << endl;
        return JS_TRUE;
    }
    return JS_FALSE;
}

static void MozEmbDestructor(JSContext *cx, JSObject *obj) {
    printf("Destroying MozEmb object\n");
    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);
    if (moz) {
        free(moz->name);
        free(moz);
    }
}

static JSBool MozEmb_load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("mozembed.load() \n");

    check_args((argc == 1), "must pass one argument!\n");
    check_args(JSVAL_IS_STRING(argv[0]), "arg must be a string (url)!");
    JSString* urlStr = JSVAL_TO_STRING(argv[0]);

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_moz_embed_load_url(moz->embed, JS_GetStringBytes(urlStr));

    return JS_TRUE;
}

static JSBool MozEmb_stop(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("object is in mainloop: %p\n", obj);

    check_args((argc == 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_moz_embed_stop_load(moz->embed);

    return JS_TRUE;
}

static JSBool MozEmb_reload(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("mozembed.load() \n");

    check_args((argc != 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    //extern void         gtk_moz_embed_reload           (GtkMozEmbed *embed, gint32 flags);
    gtk_moz_embed_reload(moz->embed, GTK_MOZ_EMBED_FLAG_RELOADNORMAL);

    return JS_TRUE;
}

/*************************************************************************************************/

///// MozEmb Function Table
static JSFunctionSpec _MozEmbFunctionSpec[] = {
    { "load", MozEmb_load, 0, 0, 0},
    { "stop", MozEmb_stop, 0, 0, 0},
    { "reload", MozEmb_reload, 0, 0, 0},
    { 0, 0, 0, 0, 0}
    /*
    http://www.mozilla.org/unix/gtk-embedding.html#function_reference

    extern void         gtk_moz_embed_load_url         (GtkMozEmbed *embed, const char *url);
    extern void         gtk_moz_embed_stop_load        (GtkMozEmbed *embed);
    extern gboolean     gtk_moz_embed_can_go_back      (GtkMozEmbed *embed);
    extern gboolean     gtk_moz_embed_can_go_forward   (GtkMozEmbed *embed);
    extern void         gtk_moz_embed_go_back          (GtkMozEmbed *embed);
    extern void         gtk_moz_embed_go_forward       (GtkMozEmbed *embed);
    extern void         gtk_moz_embed_render_data      (GtkMozEmbed *embed,
                                                        const char *data, guint32 len,
                                                        const char *base_uri, const char *mime_type);
    extern void         gtk_moz_embed_open_stream      (GtkMozEmbed *embed,
                                                        const char *base_uri, const char *mime_type);
    extern void         gtk_moz_embed_append_data      (GtkMozEmbed *embed,
                                                        const char *data, guint32 len);
    extern void         gtk_moz_embed_close_stream     (GtkMozEmbed *embed);
    extern char        *gtk_moz_embed_get_link_message (GtkMozEmbed *embed);
    extern char        *gtk_moz_embed_get_js_status    (GtkMozEmbed *embed);
    extern char        *gtk_moz_embed_get_title        (GtkMozEmbed *embed);
    extern char        *gtk_moz_embed_get_location     (GtkMozEmbed *embed);
    extern void         gtk_moz_embed_set_chrome_mask  (GtkMozEmbed *embed, guint32 flags);
    extern guint32      gtk_moz_embed_get_chrome_mask  (GtkMozEmbed *embed);
    
    static funcs:

    extern GtkType      gtk_moz_embed_get_type         (void);

    ///-> constructor
    extern GtkWidget   *gtk_moz_embed_new              (void);
    extern void         gtk_moz_embed_set_comp_path    (char *aPath);

     */


};


/*************************************************************************************************/
///// MozEmb Class Definition

static JSClass MozEmb_jsClass = {
    "MozEmb", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, // add/del prop
    JS_PropertyStub, JS_PropertyStub, // get/set prop
    JS_EnumerateStub, JS_ResolveStub, // enum / resolve
    JS_ConvertStub, MozEmbDestructor, // convert / finalize
    0, 0, 0, 0,
    0, 0, 0, 0
};
/*************************************************************************************************/

/* static funcs: */
static JSBool MozEmb_s_mainloop(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("object is in mainloop: %p\n", obj);

    check_args((argc == 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_main();

    return JS_TRUE;
}

static JSBool MozEmb_s_quit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("object is in mainloop: %p\n", obj);

    check_args((argc == 0), "must not pass an argument!\n");

    gtk_exit(0);

    return JS_TRUE;
}

static JSFunctionSpec _MozEmbStaticFunctionSpec[] = {
    { "main", MozEmb_s_mainloop, 0, 0, 0},
    { "quit", MozEmb_s_quit, 0, 0, 0},
    { 0, 0, 0, 0, 0}
};


static JSBool initEnvironment(void)
{
     int argc = 0;
    char** argv = {NULL};
   gtk_set_locale();
    gtk_init(&argc, &argv);

    cerr << "init_gtk 1a " << endl;
//    gtk_moz_embed_set_comp_path("/usr/lib/xulrunner-1.9.1.8/components/");
    cerr << "init_gtk 1b " << endl;
//    gtk_moz_embed_set_profile_path("/tmp","web_browser_user");
    cerr << "init_gtk 1c " << endl;

  gtk_moz_embed_push_startup();

#if 1

  static const GREVersionRange greVersion = {
    "1.9a", PR_TRUE,
    "2", PR_TRUE
  };

  char xpcomPath[PATH_MAX];

  nsresult rv = GRE_GetGREPathWithProperties(&greVersion, 1, nsnull, 0,
                                             xpcomPath, sizeof(xpcomPath));
  if (NS_FAILED(rv)) {
    fprintf(stderr, "Couldn't find a compatible GRE.\n");
    return 1;
  }

  rv = XPCOMGlueStartup(xpcomPath);
  if (NS_FAILED(rv)) {
    fprintf(stderr, "Couldn't start XPCOM.");
    return 1;
  }

  rv = GTKEmbedGlueStartup();
  if (NS_FAILED(rv)) {
    fprintf(stderr, "Couldn't find GTKMozEmbed symbols.");
    return 1;
  }

  char *lastSlash = strrchr(xpcomPath, '/');
  if (lastSlash)
    *lastSlash = '\0';

  gtk_moz_embed_set_path(xpcomPath);

  char *home_path;
  char *full_path;
  home_path = getenv("HOME");
  if (!home_path) {
    fprintf(stderr, "Failed to get HOME\n");
    exit(1);
  }
  full_path = g_strdup_printf("%s/%s", home_path, ".TestGtkEmbed");

  gtk_moz_embed_set_profile_path(full_path, "TestGtkEmbed");
#endif

}

///// Actor Initialization Method

JSObject* MozEmbInit(JSContext *cx, JSObject *obj) {
    if (obj == NULL)
        obj = JS_GetGlobalObject(cx);

    initEnvironment();

    JS_InitClass(cx, obj, NULL,
            &MozEmb_jsClass,
            MozEmbConstructor, 0,
            NULL, // properties
            _MozEmbFunctionSpec, // functions
            NULL, // static properties
            _MozEmbStaticFunctionSpec // static functions
            );
}

/*************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int js_DSO_load(JSContext *context) {
        if (!MozEmbInit(context, NULL)) {
            fprintf(stderr, "Cannot init DBus class\n");
            return EXIT_FAILURE;
        }

        return JS_TRUE;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

