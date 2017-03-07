CXX = g++
CXXFLAGS = -std=c++11 -std=gnu++11 -pthread
LDFLAGS = -L/usr/local/lib `pkg-config --libs grpc++` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl

all: server client 

calc.grpc.pb.cc: calc.proto
	protoc -I=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` calc.proto

calc.pb.cc: calc.proto
	protoc -I=. --cpp_out=. calc.proto

server: calc_server.cc calc.grpc.pb.cc calc.pb.cc
	$(CXX) $(CXXFLAGS) -o calc_server calc_server.cc calc.grpc.pb.cc calc.pb.cc $(LDFLAGS)

client: calc_client.cc calc.grpc.pb.cc calc.pb.cc
	$(CXX) $(CXXFLAGS) -o calc_client calc_client.cc calc.grpc.pb.cc calc.pb.cc $(LDFLAGS)

clean:
	rm *.pb.cc *.pb.h calc_server calc_client
