/**
 *  DIY SmartBlinds for SmartThings
 *
 *  Copyright 2020 Peter Chodyra
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */
metadata {
	definition (name: "DIY SmartBlinds for SmartThings", namespace: "crashtestoz", author: "Peter Chodyra", cstHandler: true, ocfDeviceType: "oic.d.blind") {
		capability "Window Shade"
        capability "Refresh"
        capability "Actuator"
		capability "Switch"
        capability "Polling"
        capability "Switch Level"   // until we get a Window Shade Level capability
        
        command "refresh"
        command "poll"
        
	}
	
    preferences {
		input("DeviceIP", "string", title:"Device IP Address", description: "Please enter your device's IP Address", required: true, displayDuringSetup: true)
		input("DevicePort", "string", title:"Device Port", description: "Please enter port 80 or your device's Port", required: true, displayDuringSetup: true)
        //input "preset", "number", title: "Preset position", description: "Set the window shade preset position", defaultValue: 50, range: "1..100", required: false, displayDuringSetup: false
	}

	simulator {
		// TODO: define status and reply messages here
	}

	tiles(scale: 2) {
        multiAttributeTile(name:"windowShade", type: "lighting", width: 6, height: 4){
            tileAttribute ("device.windowShade", key: "PRIMARY_CONTROL") {
            	attributeState "unknown", label:'${name}', action:"refresh", icon:"st.shades.shade-open", backgroundColor:"#ffffff", nextState:"opening"
                attributeState "open", label:'${name}', action:"close", icon:"st.shades.shade-open", backgroundColor:"#00a0dc", nextState:"closing"
                attributeState "closed", label:'${name}', action:"open", icon:"st.shades.shade-closed", backgroundColor:"#ffffff", nextState:"opening"
                attributeState "partially open", label:'${name}', action:"close", icon:"st.shades.shade-open", backgroundColor:"#00a0dc", nextState:"closing"
                attributeState "opening", label:'${name}', action:"", icon:"st.shades.shade-opening", backgroundColor:"#00a0dc", nextState:"partially open"
                attributeState "closing", label:'${name}', action:"", icon:"st.shades.shade-closing", backgroundColor:"#ffffff", nextState:"partially open"
            }
            //tileAttribute ("device.shadeLevel", key: "SECONDARY_CONTROL") {
        	//	attributeState "level", label:'Blinds are at\n ${currentValue} %', icon: "st.shades.shade-open"
        	//}
            //tileAttribute ("device.level", key: "SLIDER_CONTROL") {
        	//	attributeState "level", action:"switch level.setLevel"
            //}
        }

        //standardTile("home", "device.level", width: 2, height: 2, decoration: "flat") {
        //    state "default", label: "home", action:"presetPosition", icon:"st.Home.home9"
        //}
        controlTile("blindsSlider", "device.blindsSlider", "slider", height: 2, width: 2, inactiveLabel: false, range:"(0..100)", decoration: "flat") {
    		state "level", action:"switch level.setLevel"
		}
        
        standardTile("refresh", "device.refresh", width: 2, height: 2, inactiveLabel: false, decoration: "flat") {
            state "default", label:'', action:"refresh", icon:"st.secondary.refresh" //, nextState: "disabled"
            //state "disabled", label:'', action:"", icon:"st.secondary.refresh"
        }
        
//        standardTile("blindsPreset00", "device.blindsPreset00", height: 2, width: 2, inactiveLabel: false, decoration: "flat") {
//			state "off", label:"0%", action:"blindsPreset00", icon: "st.Weather.weather14", backgroundColor:"#ffffff", nextState:"at0"
//            state "at0", label:"0%", action:"", icon:"st.Weather.weather11", backgroundColor:"#00a0dc"
//		}
//        standardTile("blindsPreset01", "device.blindsPreset01", height: 2, width: 2, inactiveLabel: false, decoration: "flat") {
//			state "off", label:"50%", action:"blindsPreset01", icon: "st.Weather.weather11", backgroundColor:"#ffffff", nextState:"at50"
//            state "at50", label:"50%", action:"", icon:"st.Weather.weather11", backgroundColor:"#00a0dc"
//		}
//        standardTile("blindsPreset02", "device.blindsPreset02", height: 2, width: 2, inactiveLabel: false, decoration: "flat") {
//			state "off", label:"100%", action:"blindsPreset02", icon: "st.Weather.weather14", backgroundColor:"#ffffff", nextState:"at100"
//            state "at100", label:"100%", action:"", icon:"st.Weather.weather14", backgroundColor:"#00a0dc"
//		}

        valueTile("blindsValue", "device.blindsValue", width: 2, height: 2, inactiveLabel: false, decoration: "flat") {
        	state "default", label: 'Blinds are at\n ${currentValue} %', defaultState: true
        }
        
        main(["windowShade"])
        details(["windowShade", "blindsSlider", "blindsValue","refresh"])

    }
}

// parse events into attributes
def parse(String description) {
	//log.debug "Parsing description '${description}'"
	// TODO: handle 'windowShade' attribute
	// TODO: handle 'supportedWindowShadeCommands' attribute
    
    //Parse GET response
    def msg = parseLanMessage(description)
    def json = msg.json
    if (json == $null){
    	log.debug "GET No json response"
        //log.debug "Instead got: $msg"
    } else {
    	log.debug "GET response " + json
        def openPercent = json.position
    	//log.debug "GET response position " + openPercent
        
        //Update tiles data
    	updateTiles("$openPercent")
    }
    //logState()
    return result
}

def logState() {
	
    log.debug "blindsPreset01"
}
def initialize() {
	//configure the schedule
    runEvery15Minutes(handler)
    //refresh()
}
def poll() {
	//Check the status of the blinds
    getStatus()
}

def handler() {
	// called every 15 minutes
    log.debug "scheduled handler called at ${new Date()}"
    getStatus()
}

// handle commands
def open() {
	log.debug "open()"
	// TODO: handle 'open' command	
    moveBlinds("100")
    //updateTiles("100")
}

def close() {
	log.debug "close()"
	// TODO: handle 'close' command
    moveBlinds("0")
    //updateTiles("0")
}

def on() {
	log.debug "open()"
	// TODO: handle 'open' command	
    moveBlinds("100")
    //updateTiles("100")
}

def off() {
	log.debug "close()"
	// TODO: handle 'close' command
    moveBlinds("0")
    //updateTiles("0")
}

// handle commands
def blindsPreset00() {
	log.debug "presetPosition()"
    moveBlinds("0")
    //updateTiles("50")
}
def blindsPreset01() {
	log.debug "presetPosition()"
    moveBlinds("50")
    //updateTiles("50")
}
def blindsPreset02() {
	log.debug "presetPosition()"
    moveBlinds("100")
    //updateTiles("50")
}

def setLevel(value) {
    log.debug "setLevel(${value.inspect()})"
    Integer level = value as Integer
    if (level < 0) level = 0
    if (level > 99) level = 100
    delayBetween([
    	moveBlinds("$level")
    ],5000)
    //Update tiles data
	//updateTiles("$level")
}

def updateTiles(value) {
	//Update the app tiles 
    Integer level = value as Integer
    log.debug "updateTiles() $level"
    
    if (level > 0 && level <= 99) {
    	sendEvent(name: "windowShade", value: "partially open", isStateChange: true)
	} else if(level == 0) {
    	sendEvent(name: "windowShade", value: "closed", isStateChange: true)
	} else {
    	sendEvent(name: "windowShade", value: "open", isStateChange: true)
    } 
    
    sendEvent(name: "blindsSlider", value: "$level", unit: "%", isStateChange: true)
    sendEvent(name: "blindsValue", value: "$level", unit: "%", isStateChange: true)
}

def refresh() {
    log.debug "refresh()"
    def host = DeviceIP
	def hosthex = convertIPtoHex(host).toUpperCase()
	def porthex = convertPortToHex(DevicePort).toUpperCase()
    //Device address from SmartThings
	device.deviceNetworkId = "$hosthex:$porthex"
    def apiPath = "/api/status"
    def method = "GET"
    def headers = [:]
	headers.put("HOST", "$host:$DevicePort")
	headers.put("Content-Type", "application/json")

	try {
		sendHubCommand(new physicalgraph.device.HubAction(
    		method: method,
    		path: apiPath,
    		headers: headers
        ))
	} catch (e) {
    	log.error "GET something went wrong: $e"
	}
}


def getStatus() {
    log.debug "getStatus()"
    def host = DeviceIP
	def hosthex = convertIPtoHex(host).toUpperCase()
	def porthex = convertPortToHex(DevicePort).toUpperCase()
    //Device address from SmartThings
	device.deviceNetworkId = "$hosthex:$porthex"
    def apiPath = "/api/status"
    def method = "GET"
    def headers = [:]
	headers.put("HOST", "$host:$DevicePort")
	headers.put("Content-Type", "application/json")

	try {
		sendHubCommand(new physicalgraph.device.HubAction(
    		method: method,
    		path: apiPath,
    		headers: headers
        ))
	} catch (e) {
    	log.error "GET something went wrong: $e"
	}
}

def moveBlinds(String level) {
	log.debug "moveBlinds()"
	def host = DeviceIP
	def hosthex = convertIPtoHex(host).toUpperCase()
	def porthex = convertPortToHex(DevicePort).toUpperCase()
    //Device address from SmartThings
	device.deviceNetworkId = "$hosthex:$porthex"
	//API path
    def apiPath = "/api/blinds?open=" + level
    log.debug "API path is: $apiPath"
	//Define Headers for PUT
    def headers = [:]
	headers.put("HOST", "$host:$DevicePort")
	headers.put("Content-Type", "text/html")
    //log.debug "The Header is $headers"
	//Define method
    def method = "PUT"
    
	try {
    	sendHubCommand(new physicalgraph.device.HubAction(
			method: method,
			path: apiPath,
			headers: headers
		))
	}
	catch (Exception e) {
		log.debug "PUT something went wrong: $e"
	}
    
    updateTiles("$level")
}

// PRITAVE FUNCTIONS

private String convertIPtoHex(ipAddress) {
	String hex = ipAddress.tokenize( '.' ).collect {  String.format( '%02x', it.toInteger() ) }.join()
	//log.debug "IP address entered is $ipAddress and the converted hex code is $hex"
return hex
}
private String convertPortToHex(port) {
	String hexport = port.toString().format( '%04x', port.toInteger() )
	//log.debug hexport
return hexport
}
private Integer convertHexToInt(hex) {
	Integer.parseInt(hex,16)
}
private String convertHexToIP(hex) {
	//log.debug("Convert hex to ip: $hex")
	[convertHexToInt(hex[0..1]),convertHexToInt(hex[2..3]),convertHexToInt(hex[4..5]),convertHexToInt(hex[6..7])].join(".")
}
private getHostAddress() {
	def parts = device.deviceNetworkId.split(":")
	//log.debug device.deviceNetworkId
	def ip = convertHexToIP(parts[0])
	def port = convertHexToInt(parts[1])
	return ip + ":" + port
}