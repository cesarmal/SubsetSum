#include "ClientData.h"

ClientData::ClientData() { last_hello_in_secs = 0; }

ClientData ClientData::operator=(const ClientData &s) {
		last_hello_in_secs = s.last_hello_in_secs;
}

ClientData::~ClientData() { ; }

void ClientData::increment_hello_time() {
		last_hello_in_secs += 1;
}

void ClientData::received_hello() {
		last_hello_in_secs = 0;
}

int ClientData::get_hello_time() {
		return last_hello_in_secs;
}


