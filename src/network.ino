

void setupNetwork() {

	WiFi.mode ( WIFI_STA );
	WiFi.begin ( wifi_ssid, wifi_password );
	Serial.println ( "" );

	// Wait for connection
	while ( WiFi.status() != WL_CONNECTED ) {
		delay ( 500 );
		Serial.print ( "." );
	}

	Serial.println ( "" );
	Serial.print ( "Connected to " );
	Serial.println ( wifi_ssid );
	Serial.print ( "IP address: " );
	Serial.println ( WiFi.localIP() );

	// if ( MDNS.begin ( "esp8266" ) ) {
	// 	Serial.println ( "MDNS responder started" );
	// }

}
