
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

	for(int i = 0; i < 100000; ++i)
	{
		// create an outstation
		auto manager2 = std::make_unique<DNP3Manager>(4, ConsoleLogger::Create());
		auto channel2 = manager2->AddTCPServer("server", FILTERS, ServerAcceptMode::CloseNew, "127.0.0.1", 20000, PrintingChannelListener::Create());
		auto outstation = channel2->AddOutstation("outstation", SuccessCommandHandler::Create(), DefaultOutstationApplication::Create(), OutstationStackConfig(DatabaseSizes::AllTypes(0)));
		outstation->Enable();

		// create a master
		auto manager = std::make_unique<DNP3Manager>(4, ConsoleLogger::Create());
		auto channel = manager->AddTCPClient("client", FILTERS, ChannelRetry(TimeDuration::Seconds(0), TimeDuration::Seconds(0)), "127.0.0.1", "127.0.0.1", 20000, PrintingChannelListener::Create());
		auto master = channel->AddMaster("master", PrintingSOEHandler::Create(), asiodnp3::DefaultMasterApplication::Create(), MasterStackConfig());
		auto integrityScan = master->AddClassScan(ClassField::AllClasses(), TimeDuration::Milliseconds(1));
		master->Enable();

		// give the two sides sufficient time to connect
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		// let things shutdown in the order they are allocated
	}

	return 0;
}

