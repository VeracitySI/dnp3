/*
 * Licensed to Green Energy Corp (www.greenenergycorp.com) under one or
 * more contributor license agreements. See the NOTICE file distributed
 * with this work for additional information regarding copyright ownership.
 * Green Energy Corp licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This project was forked on 01/01/2013 by Automatak, LLC and modifications
 * may have been made to this file. Automatak, LLC licenses these modifications
 * to you under the terms of the License.
 */
#include <asiodnp3/DNP3Manager.h>
#include <asiodnp3/PrintingSOEHandler.h>
#include <asiodnp3/ConsoleLogger.h>
#include <asiodnp3/DefaultMasterApplication.h>
#include <asiodnp3/PrintingCommandCallback.h>
#include <asiodnp3/PrintingChannelListener.h>

#include <asiopal/UTCTimeSource.h>

#include <opendnp3/LogLevels.h>

using namespace std;
using namespace openpal;
using namespace asiopal;
using namespace asiodnp3;
using namespace opendnp3;

int main(int argc, char* argv[])
{
	// Specify what log levels to use. NORMAL is warning and above
	// You can add all the comms logging by uncommenting below
	const uint32_t FILTERS = levels::NORMAL | levels::ALL_APP_COMMS;

	// Operation Counters
	auto numOpen = 0;
	auto numOpenFail = 0;
	auto numClose = 0;
	auto numAttempts = 0;

        auto c_ip = std::string("0.0.0.0");
	auto c_port = atoi(std::string("20000").c_str());
	auto c_count = atoi(std::string("5").c_str());

	// Explicit addition of <IP> and <PORT>
	// arguments from the command-line.
	// The expected usage is : <program> <IP> <PORT> <COUNT>
	// Including 1 parameter will override the <IP>
	// Including 2 parameters will override the <IP> and <PORT>
	// Including 3 parameters will override the <IP> , <PORT> and <COUNT>
	// Otherwise , the defaults will be used with no
	// overrides
	if(argc==2) {
	    c_ip = std::string(argv[1]);
	}
	else if (argc == 3) {
	    c_ip = std::string(argv[1]);
	    c_port = atoi(std::string(argv[2]).c_str());
	}
        else if (argc == 4) {
	    c_ip = std::string(argv[1]);
	    c_port = atoi(std::string(argv[2]).c_str());
	    c_count = atoi(std::string(argv[3]).c_str());
	}

	std::cout<< "DNP3 master command sequence started" << std::endl;
	for(auto cnt = 0;
		 cnt < c_count;
	         cnt++)
	{		
            // This is the main point of interaction with the stack
	    DNP3Manager manager(1, ConsoleLogger::Create());

	    // Connect via a TCPClient socket to a outstation
	    auto channel = manager.AddTCPClient("tcpclient", 
			                        FILTERS, ChannelRetry::Default(), 
						c_ip , 
						"0.0.0.0", 
						c_port, 
						PrintingChannelListener::Create());

	    // The master config object for a master. The default are
	    // useable, but understanding the options are important.
	    MasterStackConfig stackConfig;

	    // you can override application layer settings for the master here
	    // in this example, we've change the application layer timeout to 2 seconds
	    stackConfig.master.responseTimeout = TimeDuration::Seconds(2);
	    stackConfig.master.disableUnsolOnStartup = true;

	    // You can override the default link layer settings here
	    // in this example we've changed the default link layer addressing
	    stackConfig.link.LocalAddr = 1;
	    stackConfig.link.RemoteAddr = 10;

	    // Create a new master on a previously declared port, with a
	    // name, log level, command acceptor, and config info. This
	    // returns a thread-safe interface used for sending commands.
	    auto master = channel->AddMaster(
	                                     "master", // id for logging
	                                     PrintingSOEHandler::Create(), // callback for data processing
	                                     asiodnp3::DefaultMasterApplication::Create(), // master application instance
	                                     stackConfig // stack configuration
	                  );


	    // do an integrity poll (Class 3/2/1/0) once per minute
	    auto integrityScan = master->AddClassScan(ClassField::AllClasses(), TimeDuration::Seconds(1));

	    // do a Class 1 exception poll every 5 seconds
	    auto exceptionScan = master->AddClassScan(ClassField(ClassField::CLASS_1), TimeDuration::Seconds(1));

	    // Enable the master. This will start communications.
	    master->Enable();

	    auto linkStatistics = channel->GetStatistics();
	    numAttempts += 1;
	    numOpen     += linkStatistics.channel.numOpen;
	    numOpenFail += linkStatistics.channel.numOpenFail;
	    numClose    += linkStatistics.channel.numClose;

	    manager.Shutdown();
            

        }

        std::cout << "attempts: " << numAttempts  << " "
	          << "open: " << numOpen << " " 
		  << "fail: " << numOpenFail << " " 
		  << "close:" << numClose << " " 
		  << std::endl;

	return 0;
}
