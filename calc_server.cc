// Implement the calc server
#include <iostream>
#include <memory>
#include <cmath>

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include "calc.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using calc::Calc;
using calc::Point;
using calc::Circle;
using calc::Area;
using calc::Circum;

// The server
class CalcImpl final : public Calc::Service {
public:
	CalcImpl() {}

	Status CalcArea(ServerContext* context, const Circle* circle, Area* area) override {
		double r = circle->radius();
		area->set_value(3.14159 * r * r);
		return Status::OK;
	}

	Status CalcCircum(ServerContext* context, const Circle* circle, Circum* circum) override {
		double r = circle->radius();
		circum->set_value(2 * 3.14159 * r);
		return Status::OK;
	}

	Status CalcAreaSum(ServerContext* context, ServerReader<Circle>* circles, Area* area) override {
		Circle circle;
		double sum = 0;
		while (circles->Read(&circle)){
			double r = circle.radius();
			sum += 3.14159 * r * r;
		}
		area->set_value(sum);
		return Status::OK;
	}

	Status CalcAreaStream(ServerContext* context, ServerReaderWriter<Area, Circle>* stream) override {
		Circle circle;
		while (stream->Read(&circle)){
			double r = circle.radius();
			Area area;
			area.set_value(3.14159 * r * r);
			stream->Write(area);
		}
		return Status::OK;
	}
};

void RunServer(){
	std::string server_address("127.0.0.1:50051");
	CalcImpl service;

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

int main(){
	RunServer();

	return 0;
}
