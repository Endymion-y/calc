// Calc client
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "calc.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using calc::Calc;
using calc::Point;
using calc::Circle;
using calc::Area;
using calc::Circum;

class CalcClient {
public:
	CalcClient(std::shared_ptr<Channel> channel) : 
		stub_(Calc::NewStub(channel)) {}

	void CalcArea(const Circle& circle, Area* area){
		ClientContext context;
		Status status = stub_->CalcArea(&context, circle, area);
		if (!status.ok()) {
			std::cout << "CalcArea rpc failed." << std::endl;
			return;
		}
		std::cout << "Calculated area: " << area->value() << std::endl;
		return;
	}

	void CalcCircum(const Circle& circle, Circum* circum){
		ClientContext context;
		Status status = stub_->CalcCircum(&context, circle, circum);
		if (!status.ok()) {
			std::cout << "CalcCircum rpc failed." << std::endl;
			return;
		}
		std::cout << "Calculated circum: " << circum->value() << std::endl;
		return;
	}

	void CalcAreaSum(){
		Circle circle;
		Area sum;
		ClientContext context;
		const int kCircles = 10;

		std::unique_ptr<ClientWriter<Circle> > writer(
				stub_->CalcAreaSum(&context, &sum));
		for (int i = 0; i < kCircles; i++){
			circle.set_radius(i + 1);
			if (!writer->Write(circle)){
				// Broken stream
				break;
			}
		}
		writer->WritesDone();
		Status status = writer->Finish();
		if (status.ok()) {
			std::cout << "Sum = " << sum.value() << std::endl;
		} else {
			std::cout << "CalcAreaSum rpc failed." << std::endl;
		}
	}

	void CalcAreaStream(){
		ClientContext context;

		std::shared_ptr<ClientReaderWriter<Circle, Area> > stream(
				stub_->CalcAreaStream(&context));

		// A new thread
		std::thread writer([stream]() {
				Circle circle;
				for (int i = 1; i <= 10; i++){
					circle.set_radius(i);
					std::cout << "Radius = " << i << std::endl;
					stream->Write(circle);
				}
				stream->WritesDone();
				});
		Area area;
		while (stream->Read(&area)){
			std::cout << "Area = " << area.value() << std::endl;
		}
		// Run thread
		writer.join();
		Status status = stream->Finish();
		if (!status.ok()) {
			std::cout << "CalcAreaStream rpc failed." << std::endl;
		}
	}

private:
	std::unique_ptr<Calc::Stub> stub_;
};

int main(int argc, char** argv){
	CalcClient client(grpc::CreateChannel("localhost:50051", 
				grpc::InsecureChannelCredentials()));
	std::cout << "---------------- CalcArea ----------------" << std::endl;
	Circle circle;
	circle.set_radius(3);
	std::cout << "radius = " << circle.radius() << std::endl;
	Area area;
	client.CalcArea(circle, &area);
	std::cout << "area = " << area.value() << std::endl;

	std::cout << "-------------- CalcAreaSum --------------" << std::endl;
	client.CalcAreaSum();
	std::cout << "------------- CalcAreaStream ------------" << std::endl;
	client.CalcAreaStream();

	return 0;
}
