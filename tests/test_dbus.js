/**
	this file shall test the functionality for exporting interfaces on the session bus.   
 */
DSO.load("dbus"); 
DSO.load("mozembed");

LIBDIR = "/opt/smarthome-src/shc/js/mozemb_js/tests/";

load(LIBDIR + "introspectable.js");


function Browser() {
	this.moz = new MozEmb();

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
						<method name="fullscreen">
						</method>
					  </interface>;

	this.goto = function(url) {
		this.moz.load(url);
	}

	this.stop = function() {
		this.moz.stop();
	}

	this.reload = function() {
		this.moz.reload();
	}

	this.fullscreen = function() {
		this.moz.fullscreen();
	}

	this.quit = function() {
		Glib.quit();
	}
}


bus = DBus.sessionBus();

intro = new Introspectable(bus);
bus._intro = intro;
intro.expose("/", intro);
bus.export("/", intro._iface, intro); 


browser = new Browser();
intro.expose("/", browser);
browser.goto("http://www.orf.at/");

bus.requestBusName("at.beko.browser");

MozEmb.main();

