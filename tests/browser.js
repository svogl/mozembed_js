/**
	this file shall test the functionality for exporting interfaces on the session bus.   
 */
LIB = "/opt/smarthome/lib/js/";

load(LIB + "common.js");
load(LIB + "shc_gui_client.js");

DSO.load("dbus"); 
DSO.load("mozembed");


function Browser(bus) {
	this.bus = bus;
	this.moz = new MozEmbed();

	this._iface = "at.beko.Browser";
	this._sig =   <interface name="at.beko.Browser">
						<method name="goto">
						  <arg name="url" type="s" direction="in"/>
						</method>
						<method name="stop">
						</method>
						<method name="reload">
						</method>
						<method name="quit">
						</method>
						<method name="back">
						</method>
						<method name="forward">
						</method>
						<method name="fullscreen">
						</method>
					  </interface>;

	this.goto = function(opath, url) {
		print("JS::Browser::load("+url+")");
		this.moz.load(url);
	}

	this.stop = function(opath) {
		print("JS::Browser::stop()");
		this.moz.stop();
	}

	this.reload = function(opath) {
		print("JS::Browser::reload()");
		this.moz.reload();
	}

	this.fullscreen = function(opath) {
		print("JS::Browser::fullscreen()");
		this.moz.fullscreen();
	}

	this.quit = function(opath) {
		print("JS::Browser::quit() " + opath);
		//Glib.quit();
		//this.moz.quit();
		MozEmbed.quit();
	}
}

///////////////////gui client:

function Client(browser) {
	this.browser = browser;

	this.play = function(arg) {
		if (arg != null && arg[0] != null ) {
			if ( arg[0]._url != null) {
				this.browser.goto("/","" + arg[0]._url);
			} else {
				this.browser.goto("/", arg);
			}
		}
	}

	this.pause = function(arg) {
		print("JS::Client::pause()");
		print("paused browser " + arg);
	}

	this.quit = function(arg) {
		print("JS::Client::quit() hier mit " + arg);
		MozEmbed.quit();

		return DBUS_COMMAND_OK;
	}
}


bus = DBus.sessionBus();

intro = new Introspectable(bus);
 function handle(str) {
		print("JS::Client::handle("+str+")");
	} 

browser = new Browser(bus);

function statusCb(str) {
	print("JS::Client::statusCb() " + str );
	if (str == "urn:/cmd/gui/quit") {
		MozEmbed.quit();
	}
}
browser.moz.setCallback(statusCb);


print("Browser.statusCb " + browser.statusCb);

intro.expose("/control", browser);

guic = new GuiClient(bus);
intro.expose(guic._path, guic);

client = new Client(browser);
guic.addClient(client);

//browser.fullscreen();
browser.goto("/","file:///opt/smarthome-src/shc/js/mozemb_js/tests/close.html");

bus.requestBusName("at.beko.smarthome.Browser.Service");

MozEmbed.main();

