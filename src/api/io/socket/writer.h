#ifndef WRITER
#define WRITER

namespace IO
{
template <typename Socket, typename DataType> class SocketWriter
{
	const Socket *sock;

	public:
	SocketWriter() = delete;
	SocketWriter(const Socket &sock) : sock(&sock) {}

	std::size_t operator()(const DataType &data)
	{
		if (sock != nullptr) {
			auto written = sock->WriteSome(data);
			return written;
		}
		return 0;
	}
};
}
#endif // WRITER
