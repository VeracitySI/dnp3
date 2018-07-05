
#include <thread>
#include <asiodnp3/DNP3Manager.h>
#include <asiodnp3/PrintingSOEHandler.h>
#include <asiodnp3/ConsoleLogger.h>
#include <asiodnp3/DefaultMasterApplication.h>
#include <asiodnp3/PrintingCommandCallback.h>
#include <asiodnp3/PrintingChannelListener.h>
#include <opendnp3/outstation/SimpleCommandHandler.h>
#include <asiopal/UTCTimeSource.h>
#include <opendnp3/LogLevels.h>

using namespace openpal;
using namespace asiopal;
using namespace asiodnp3;
using namespace opendnp3;

int main()
{
	const uint32_t FILTERS = levels::NOTHING;

	while(1)
	{
		auto manager2 = std::make_unique<DNP3Manager>(3, ConsoleLogger::Create());
		auto channel2 = manager2->AddTCPServer("server", FILTERS, ServerAcceptMode::CloseNew, "127.0.0.1", 20000, PrintingChannelListener::Create());
		OutstationStackConfig config(DatabaseSizes::AllTypes(1));
		config.link.LocalAddr = 10;
		config.link.RemoteAddr = 1;
		auto outstation = channel2->AddOutstation("outstation", SuccessCommandHandler::Create(), DefaultOutstationApplication::Create(), config);
		outstation->Enable();

		auto manager = std::make_unique<DNP3Manager>(3, ConsoleLogger::Create());
		auto channel = manager->AddTCPClient("tcpclient", FILTERS, ChannelRetry(TimeDuration::Seconds(0), TimeDuration::Seconds(0)),
								"127.0.0.1", "127.0.0.1", 20000, PrintingChannelListener::Create());
		MasterStackConfig stackConfig;
		stackConfig.link.LocalAddr = 1;
		stackConfig.link.RemoteAddr = 10;
		auto master = channel->AddMaster("master",PrintingSOEHandler::Create(),asiodnp3::DefaultMasterApplication::Create(),stackConfig);
		auto integrityScan = master->AddClassScan(ClassField::AllClasses(), TimeDuration::Milliseconds(1));
		master->Enable();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		integrityScan.reset();
		master.reset();
		channel.reset();
		manager.reset();

		outstation.reset();
		channel2.reset();
		manager2.reset();
	}

	return 0;
}

