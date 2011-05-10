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

#include <nsIDocShell.h>
#include <nsCOMPtr.h>
#include <nsIContentViewer.h>
#include <nsIWebBrowser.h>
#include <nsIDOMWindow.h>
#include <nsIMarkupDocumentViewer.h>
#include <nsIInterfaceRequestor.h>
#include <nsIInterfaceRequestorUtils.h>

#include <nsXPCOMGlue.h>

#define fail_if_not(assert, ...) if (!(assert)) { \
		fprintf(stderr, "%s:%d :: ", __FUNCTION__, __LINE__ );\
		fprintf(stderr, __VA_ARGS__);\
		return JS_FALSE; \
	}

#define fail_if(assert, ...) fail_if_not (!(assert), __VA_ARGS__)

using namespace std;

/*
 * GetDOMOWindowObject: 
 *		retrieve refercne to the DOMWindow
 *
 */
static nsIDOMWindow *GetDOMOWindowObject(nsIWebBrowser * wb)
{
    nsCOMPtr < nsIDOMWindow > oDomWindow;
    nsresult rv;

    rv = wb->GetContentDOMWindow(getter_AddRefs(oDomWindow));

   // BR_LOGPRINTF("GetContentDOMWindow rv %d", rv);

    if (!oDomWindow)
    {
     //   BR_WARNPRINTF("Cannot create Dom Window Object");
    }
    return oDomWindow;
}


static JSBool __browser_set_zoom(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval,nsIWebBrowser* browser ) 
{
#if 0
    nsCOMPtr < nsIDOMWindow > activeWindow;
	// only works for text...
    activeWindow = GetDOMOWindowObject(browser);

    if ((activeWindow))
    {
        activeWindow->SetTextZoom(1.41f);
        return TRUE;
    }
#endif    
	nsCOMPtr < nsIDocShell > doc_shell;
    doc_shell = do_GetInterface(browser);
	if (!doc_shell) {
		cerr << "*** docshell returned null" << endl;
		return JS_FALSE;
	}

    nsIContentViewer* contentView;
	doc_shell->GetContentViewer(&contentView);

	nsCOMPtr<nsIMarkupDocumentViewer> markp;
	static const nsIID iid3 = NS_IMARKUPDOCUMENTVIEWER_IID;
	contentView->QueryInterface( iid3, getter_AddRefs(markp) );

	markp->SetFullZoom(1.41f);

}


#include "gtkmozembed.h"
#include "gtkmozembed_internal.h"

////////////////////////////////////////
/////   MozEmb class
////////////////////////////////////////

typedef struct _MozEmb {
    // my data;
    GtkWidget *window;
    GtkMozEmbed *embed;
    char* name; ///< widget?
    JSContext *cx; ///< our js object
    JSObject *obj; ///< self
    JSFunction *cb; ///< callback function, if any
} MozEmbData;

/*************************************************************************************************/

static JSBool MozEmb_set_zoom(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);
    cerr << "set zoom!" << endl;

    nsCOMPtr<nsIWebBrowser> brws;
    nsCOMPtr<nsIDOMWindow>  domWindow;

    gtk_moz_embed_get_nsIWebBrowser(GTK_MOZ_EMBED(moz->embed), getter_AddRefs(brws));
	if (!brws) {
		cerr << "browser interface not found " << endl;
		return JS_FALSE;
	}
	nsIWebBrowser* browser = brws;
	
#if 0
	// only works for text...
    nsCOMPtr < nsIDOMWindow > activeWindow;
    activeWindow = GetDOMOWindowObject(browser);

    if ((activeWindow))
    {
        activeWindow->SetTextZoom(1.41f);
        return TRUE;
    }
#endif    

	nsCOMPtr < nsIDocShell > doc_shell;
    doc_shell = do_GetInterface(browser);
	if (!doc_shell) {
		cerr << "*** docshell returned null" << endl;
		return JS_FALSE;
	}

    nsIContentViewer* contentView;
	doc_shell->GetContentViewer(&contentView);

	nsCOMPtr<nsIMarkupDocumentViewer> markp;
	static const nsIID iid3 = NS_IMARKUPDOCUMENTVIEWER_IID;
	contentView->QueryInterface( iid3, getter_AddRefs(markp) );

	markp->SetFullZoom(1.41f);

	return JS_TRUE;
}

static void link_message_cb(GtkMozEmbed *embed, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;

    cout << "link_message_cb callback!";
    char* status = gtk_moz_embed_get_link_message(embed);
    cout << status << endl;
    g_free(status);

    JS_BeginRequest(moz->cx);
    JS_EndRequest(moz->cx);
}


/** trigger the js callback function using a string parameter. n.b. that the strings is copied.
 */
static void _triggerCallbackStr(MozEmbData* moz, const char* source, const char* arg)
{
    JS_BeginRequest(moz->cx);

    jsval args[2];
    jsval ret;
    args[0] = STRING_TO_JSVAL( JS_NewStringCopyZ(moz->cx, source) );
    args[1] = STRING_TO_JSVAL( JS_NewStringCopyZ(moz->cx, arg) );

    JSBool ok = JS_CallFunction(moz->cx, moz->obj, moz->cb, 2, args, &ret);

    if (!ok) {
      cerr << "Error: Callback for " << source << " did not work out!" << endl;
    }

    JS_EndRequest(moz->cx);
}

/** second argument is set to null!
 */
static void _triggerCallback(MozEmbData* moz, const char* source)
{
    JS_BeginRequest(moz->cx);

    jsval args[2];
    jsval ret;
    args[0] = STRING_TO_JSVAL( JS_NewStringCopyZ(moz->cx, source) );
    args[1] = JSVAL_NULL;

    JSBool ok = JS_CallFunction(moz->cx, moz->obj, moz->cb, 2, args, &ret);

    if (!ok) {
      cerr << "Error: Callback for " << source << " did not work out!" << endl;
    }

    JS_EndRequest(moz->cx);
}

/**
 */
static void _triggerCallbackDouble(MozEmbData* moz, const char* source, jsdouble arg)
{
    JS_BeginRequest(moz->cx);

    jsval args[2];
    jsval ret;
    args[0] = STRING_TO_JSVAL( JS_NewStringCopyZ(moz->cx, source) );
    args[1] = DOUBLE_TO_JSVAL( &arg );

    JSBool ok = JS_CallFunction(moz->cx, moz->obj, moz->cb, 2, args, &ret);

    if (!ok) {
      cerr << "Error: Callback for " << source << " did not work out!" << endl;
    }

    JS_EndRequest(moz->cx);
}

/**
 */
static void _triggerCallbackInt(MozEmbData* moz, const char* source, jsint arg)
{
    JS_BeginRequest(moz->cx);

    jsval args[2];
    jsval ret;
    args[0] = STRING_TO_JSVAL( JS_NewStringCopyZ(moz->cx, source) );
    args[1] = INT_TO_JSVAL( arg );

    JSBool ok = JS_CallFunction(moz->cx, moz->obj, moz->cb, 2, args, &ret);

    if (!ok) {
      cerr << "Error: Callback for " << source << " did not work out!" << endl;
    }

    JS_EndRequest(moz->cx);
}

static void js_status_cb(GtkMozEmbed *embed, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;

    char* status = gtk_moz_embed_get_js_status(embed);
    cout << "js_status_c callback!" << status << endl;

    if (moz->cb == 0 ) {
        cout << "js_status no cb\n";
        return;
    }

    _triggerCallbackStr(moz,"status",status);
    g_free(status);
}

static void location_changed_cb(GtkMozEmbed *embed, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;
    char* url = gtk_moz_embed_get_location(moz->embed);
    cout << "location_changed_cb callback!" << endl;
    _triggerCallbackStr(moz,"location_changed",url);
    g_free(url);
}

static void title_changed_cb(GtkMozEmbed *embed, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;

    cout << "title_changed_cb callback!" << endl;
    _triggerCallbackStr(moz,"title_changed","");
}

static void progress_change_cb(GtkMozEmbed *embed, gint cur, gint max, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;

    cout << "progress_change_cb callback!" << cur << "/" << max << endl;
    gtk_main_iteration();
    _triggerCallbackDouble(moz,"progress", cur/max);
}

static void net_state_change_cb(GtkMozEmbed *embed, gint flags, guint status, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;

    cout << "net_state_change_cb callback! " << status << endl;
    _triggerCallbackInt(moz,"state_changed",status);
}

static void load_started_cb(GtkMozEmbed *embed, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;

    cout << "load_started_cb callback!" << endl;
    _triggerCallback(moz,"load_started");
}

static void load_finished_cb(GtkMozEmbed *embed, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;

    cout << "load_finished_cb callback!" << endl;
    _triggerCallback(moz,"load_finished");
}

static void new_window_cb(GtkMozEmbed *embed, GtkMozEmbed **retval, guint chromemask, gpointer data) {
//    MozEmbData* moz = (MozEmbData*) data;

    cout << "new_window_cb callback!" << hex << chromemask << dec << endl;
    *retval = 0;
}

static void visibility_cb(GtkMozEmbed *embed, gboolean visibility, gpointer data) {
//    MozEmbData* moz = (MozEmbData*) data;
    cout << "visibility_cb callback!" << endl;
}

static void destroy_brsr_cb(GtkMozEmbed *embed, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;
    cerr << "destroy_brsr_cb callback!" << endl;

    JS_BeginRequest(moz->cx);
    jsval args;
    jsval ret;
    JSBool ok = JS_CallFunctionName(moz->cx, moz->obj, "quitCb", 0, &args, &ret);
    if (!ok) {
        cerr << "Error: Callback did not work out!" << endl;
    }
    JS_EndRequest(moz->cx);
}

static gint destroy_cb(GtkMozEmbed *embed, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;
    cout << "destroy callback!" << endl;
}

static gint open_uri_cb(GtkMozEmbed *embed, const char *uri, gpointer data) {
    MozEmbData* moz = (MozEmbData*) data;
    cout << "open_uri_cb callback!" << endl;
}

static void init_gtk_stuff(MozEmbData* moz) {
    int argc = 0;
    char** argv = {NULL};
    cerr << "init_gtk 1 " << endl;

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    cerr << "init_gtk 1d " << endl;
    GtkWidget* m = gtk_moz_embed_new();

    gtk_widget_set_size_request(m, 800, 600);

    gtk_container_add(GTK_CONTAINER(window), m);

    int ret = 0;

    g_signal_connect(m, "net_stop",
            (GCallback) load_finished_cb, (gpointer) moz);
    cerr << "ret " << ret << endl;
    g_signal_connect(GTK_OBJECT(m), "net_start",
            G_CALLBACK(load_started_cb), (gpointer) moz);
    cerr << "ret " << ret << endl;
    g_signal_connect(m, "net_state",
            (GCallback) net_state_change_cb, (gpointer) moz);
    cerr << "ret " << ret << endl;
    g_signal_connect(m, "progress",
            (GCallback) progress_change_cb, (gpointer) moz);
    cerr << "ret " << ret << endl;
    g_signal_connect(m, "location",
            (GCallback) location_changed_cb, (gpointer) moz);
    cerr << "ret " << ret << endl;

    g_signal_connect(m, "js_status",
            (GCallback) js_status_cb, (gpointer) moz);
    cerr << "ret " << ret << endl;

    ret = g_signal_connect(GTK_OBJECT(m), "destroy",
            G_CALLBACK(destroy_cb), moz);
    cerr << "ret d " << ret << endl;

    ret = g_signal_connect(GTK_OBJECT(window), "destroy",
            G_CALLBACK(destroy_cb), moz);
    cerr << "ret w " << ret << endl;

    ret = g_signal_connect(m, "new_window",
            (GCallback) new_window_cb, (gpointer) moz);
    cerr << "ret n " << ret << endl;

    ret = g_signal_connect(GTK_OBJECT(m), "destroy_browser",
            G_CALLBACK(destroy_brsr_cb), (gpointer) moz);
    cerr << "ret x " << ret << endl;

    gtk_window_set_decorated(GTK_WINDOW(window), 0);

    gtk_widget_show(m);
    gtk_widget_show(window);
	gtk_widget_show_all(window);
    moz->embed = GTK_MOZ_EMBED(m);
    moz->window = window;
}

/** MozEmb Constructor */
static JSBool MozEmbConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    cerr << "*Moz 1 " << endl;
    if (argc == 0) {
        MozEmbData* moz = NULL;

        moz = (MozEmbData*) calloc(sizeof (MozEmbData), 1);
        moz->cx = cx;
        moz->obj = obj;
        moz->cb = 0;
        
        if (!JS_SetPrivate(cx, obj, moz))
            return JS_FALSE;

        init_gtk_stuff(moz);
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

static JSBool MozEmb_set_callback(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("object is in mainloop: %p\n", obj);

    fail_if((argc != 1), "must pass one argument ( callback(str) )!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    if (!JSVAL_IS_OBJECT(argv[0])) {
        cerr << "func no obj"<<endl;
        return JS_FALSE;
    }
    JSObject * func = JSVAL_TO_OBJECT(argv[0]);
    if (!JS_ObjectIsFunction(moz->cx, func)) {
        cerr << "obj is no func"<<endl;
        return JS_FALSE;
    };

    JSFunction* cb = JS_ValueToFunction(moz->cx, argv[0]);
    moz->cb = cb;

    return JS_TRUE;
}

static JSBool MozEmb_load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("mozembed.load() \n");

    fail_if((argc != 1), "must pass one argument!\n");
    fail_if_not(JSVAL_IS_STRING(argv[0]), "arg must be a string (url)!");
    JSString* urlStr = JSVAL_TO_STRING(argv[0]);

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_moz_embed_load_url(moz->embed, JS_GetStringBytes(urlStr));

    gint w, h;
    gtk_window_get_size(GTK_WINDOW(moz->window), &w, &h);
    gtk_window_resize(GTK_WINDOW(moz->window), w, h);

    return JS_TRUE;
}

static JSBool MozEmb_stop(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("object is in mainloop: %p\n", obj);

    fail_if_not((argc == 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_moz_embed_stop_load(moz->embed);

    return JS_TRUE;
}

static JSBool MozEmb_go_back(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("mozembed.go_back() \n");

    fail_if((argc != 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_moz_embed_go_back(moz->embed);

    return JS_TRUE;
}

static JSBool MozEmb_go_forward(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    fail_if((argc != 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_moz_embed_can_go_forward(moz->embed);

    return JS_TRUE;
}

static JSBool MozEmb_reload(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("mozembed.reload() \n");

    fail_if((argc != 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_moz_embed_reload(moz->embed, GTK_MOZ_EMBED_FLAG_RELOADNORMAL);

    return JS_TRUE;
}

static JSBool GtkWin_fullscreen(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("mozembed.load() \n");

    fail_if((argc != 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_window_fullscreen(GTK_WINDOW(moz->window));

    return JS_TRUE;
}

static JSBool GtkWin_unfullscreen(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    fail_if((argc != 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_window_unfullscreen(GTK_WINDOW(moz->window));

    return JS_TRUE;
}

static JSBool MozEmb_get_title(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("mozembed.get_title() \n");

    fail_if((argc != 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);
    char* title;

    title = gtk_moz_embed_get_title(moz->embed);
    *rval = STRING_TO_JSVAL( JS_NewString(moz->cx, title, strlen(title)) );
    return JS_TRUE;
}

static JSBool MozEmb_get_location(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("mozembed.get_title() \n");

    fail_if((argc != 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);
    char* title;

    title = gtk_moz_embed_get_location(moz->embed);
    *rval = STRING_TO_JSVAL( JS_NewString(moz->cx, title, strlen(title)) );
    return JS_TRUE;
}


/*************************************************************************************************/

///// MozEmb Function Table
static JSFunctionSpec _MozEmbFunctionSpec[] = {
    { "load", MozEmb_load, 0, 0, 0},
    { "stop", MozEmb_stop, 0, 0, 0},
    { "back", MozEmb_go_back, 0, 0, 0},
    { "forward", MozEmb_go_forward, 0, 0, 0},
    { "reload", MozEmb_reload, 0, 0, 0},
    { "zoom", MozEmb_set_zoom, 0, 0, 0},
    { "setCallback", MozEmb_set_callback, 0, 0, 0},
    { "fullscreen", GtkWin_fullscreen, 0, 0, 0},
    { "unfullscreen", GtkWin_unfullscreen, 0, 0, 0},
    // informational funcs:
    { "title", MozEmb_get_title, 0, 0, 0},
    { "location", MozEmb_get_location, 0, 0, 0},
    { 0, 0, 0, 0, 0}
    /*
    http://www.mozilla.org/unix/gtk-embedding.html#function_reference

    extern gboolean     gtk_moz_embed_can_go_back      (GtkMozEmbed *embed);
    extern gboolean     gtk_moz_embed_can_go_forward   (GtkMozEmbed *embed);
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
     */
};


/*************************************************************************************************/
///// MozEmb Class Definition

static JSClass MozEmb_jsClass = {
    "MozEmbed", JSCLASS_HAS_PRIVATE,
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

    fail_if_not((argc == 0), "must not pass an argument!\n");

    MozEmbData* moz = (MozEmbData *) JS_GetPrivate(cx, obj);

    gtk_moz_embed_push_startup();
    gtk_main();
    gtk_moz_embed_pop_startup();

    return JS_TRUE;
}

static JSBool MozEmb_s_quit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    printf("object is in mainloop: %p\n", obj);

    fail_if_not((argc == 0), "must not pass an argument!\n");

    gtk_exit(0);

    return JS_TRUE;
}

static JSFunctionSpec _MozEmbStaticFunctionSpec[] = {
    { "main", MozEmb_s_mainloop, 0, 0, 0},
    { "quit", MozEmb_s_quit, 0, 0, 0},
    { 0, 0, 0, 0, 0}
};

static GtkMozEmbedSingle *single = 0;

void new_window_orphan_cb(GtkMozEmbedSingle *embed,
        GtkMozEmbed **retval, guint chromemask,
        gpointer data) {
    g_print("new_window_orphan_cb\n");
    g_print("chromemask is %d\n", chromemask);
    //  TestGtkBrowser *newBrowser = new_gtk_browser(chromemask);
    //  *retval = GTK_MOZ_EMBED(newBrowser->mozEmbed);
    *retval = 0;
    g_print("new browser is %p\n", (void *) * retval);
}

#include <gtkmozembed_glue.cpp>

static JSBool initEnvironment(void) {
    int argc = 0;
    char** argv = {NULL};
    gtk_set_locale();
    gtk_init(&argc, &argv);

    cerr << "initEnv 10a " << endl;

    static const GREVersionRange greVersion = {
        "1.9a", PR_TRUE,
        "2", PR_TRUE
    };

    char xpcomPath[PATH_MAX];

    nsresult rv = GRE_GetGREPathWithProperties(&greVersion, 1, nsnull, 0,
            xpcomPath, sizeof (xpcomPath));
    if (NS_FAILED(rv)) {
        fprintf(stderr, "Couldn't find a compatible GRE.\n");
        return 1;
    }
    cerr << "initEnv 2a " << endl;

    rv = XPCOMGlueStartup(xpcomPath);
    if (NS_FAILED(rv)) {
        fprintf(stderr, "Couldn't start XPCOM.");
        return 1;
    }
    cerr << "initEnv 3a " << endl;

    rv = GTKEmbedGlueStartup();
    if (NS_FAILED(rv)) {
        fprintf(stderr, "Couldn't find GTKMozEmbed symbols.");
        return 1;
    }
    cerr << "initEnv 4a " << endl;

    rv = GTKEmbedGlueStartupInternal();

    char *lastSlash = strrchr(xpcomPath, '/');
    if (lastSlash)
        *lastSlash = '\0';

    cerr << "initEnv 5a " << xpcomPath << endl;

    gtk_moz_embed_set_path(xpcomPath);

    char *home_path;
    char *full_path;
    home_path = getenv("HOME");
    if (!home_path) {
        fprintf(stderr, "Failed to get HOME\n");
        exit(1);
    }
    full_path = g_strdup_printf("%s/%s", home_path, ".TestGtkEmbed");

    cerr << "initEnv 6a " << full_path << endl;

    gtk_moz_embed_set_profile_path(full_path, "TestGtkEmbed");

    single = gtk_moz_embed_single_get();
    if (!single) {
        fprintf(stderr, "Failed to get singleton embed object!\n");
        exit(1);
    }

    g_signal_connect(GTK_OBJECT(single), "new_window_orphan",
            G_CALLBACK(new_window_orphan_cb), NULL);


    cerr << "initEnv fin " << endl;

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

