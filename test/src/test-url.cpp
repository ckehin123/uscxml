#include "uscxml/URL.h"
#include "uscxml/Message.h"
#include "uscxml/Interpreter.h"
#include "uscxml/server/HTTPServer.h"

#include "uscxml/config.h"

#include <SAX/helpers/InputSourceResolver.hpp>

#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <iostream>

using namespace uscxml;
using namespace boost;

class TestServlet : public HTTPServlet {
public:
	TestServlet(bool adaptPath) : _canAdaptPath(adaptPath) {}

	bool httpRecvRequest(const HTTPServer::Request& request) {
		return true;
	};
	bool canAdaptPath() {
		return _canAdaptPath;
	}
	void setURL(const std::string& url) {
		_actualUrl = url;
	}

	std::string _actualUrl;
	bool _canAdaptPath;
};

bool canResolve(const std::string& url) {
	Arabica::SAX::InputSource<std::string> is(url);
	Arabica::SAX::InputSourceResolver res1(is, Arabica::default_string_adaptor<std::string>());
	if(res1.resolve()) {
		std::cout << "good: " << url << std::endl;
		return true;
	} else {
		std::cout << "bad:  " << url << std::endl;
		return false;
	}
}

void testFileURLs() {

	// absolute
	std::list<URL> absURLs;
	{
		// with explicit file schema
		absURLs.push_back(URL("file:///"));
		absURLs.push_back(URL("file:/"));

		// platform specific
		absURLs.push_back(URL("file:/Z:/Windows/workspace/uscxml/bin/com/carmeq/scxml/test-xml-access.xml"));
		absURLs.push_back(URL("file:/C:/Windows/config.sys"));
		absURLs.push_back(URL("file:/Macintosh%20HD/fileURLs/text.txt"));
		absURLs.push_back(URL("file:/fileURLs/text.txt"));

		// usual filesystem paths
		absURLs.push_back(URL("C:\\Windows\\sradomski\\Desktop\\foo.txt"));
//		absURLs.push_back(URL("C:\\Windows\\Some Spaces\\index.txt"));
//		absURLs.push_back(URL("C:/Windows/Some Spaces/index.txt"));
//		absURLs.push_back(URL("/Users/sradomski/Desktop/"));
//		absURLs.push_back(URL("/Users/sradomski/Desktop/foo.txt"));
	}
	
	std::list<URL> absWithHostURLs;
	{
		absWithHostURLs.push_back(URL("file://hostname/"));
		absWithHostURLs.push_back(URL("file://localhost/"));
		absWithHostURLs.push_back(URL("file://C:/config.sys"));
		absWithHostURLs.push_back(URL("file://config.sys"));
		absWithHostURLs.push_back(URL("file://config.sys"));
		absWithHostURLs.push_back(URL("file://Macintosh%20HD/fileURLs/text.txt"));
		absWithHostURLs.push_back(URL("file://fileURLs/text.txt"));
		absWithHostURLs.push_back(URL("file://Desktop/workspace/uscxml/bin/com/carmeq/scxml/test-xml-access.xml"));
		absWithHostURLs.push_back(URL("file://Windows\\sradomski\\Desktop\\foo.txt"));

	}
	// relative URLs
	std::list<URL> relURLs;
	
	{
		relURLs.push_back(URL("file"));
		relURLs.push_back(URL("file:"));
		relURLs.push_back(URL("file://"));
		
		// platform specific
		relURLs.push_back(URL("file:Macintosh%20HD/fileURLs/text.txt"));
		relURLs.push_back(URL("file:fileURLs/text.txt"));
		relURLs.push_back(URL("file:Document/Text.foo"));
		
		// usual filesystem paths
		relURLs.push_back(URL("Users\\sradomski\\Desktop\\foo.txt"));
		relURLs.push_back(URL("Document\\Some Spaces\\index.txt"));
		relURLs.push_back(URL("Document/Some Spaces/index.txt"));
		relURLs.push_back(URL("Users/sradomski/Desktop/"));
		relURLs.push_back(URL("Users/sradomski/Desktop/foo.txt"));
	}

	for (std::list<URL>::iterator absIter = absURLs.begin(); absIter != absURLs.end(); absIter++) {
		absIter->dump();
		assert(absIter->isAbsolute());
		assert(absIter->scheme() == "file");
		assert(absIter->host() == "");
	}

	for (std::list<URL>::iterator relIter = relURLs.begin(); relIter != relURLs.end(); relIter++) {
		assert(!relIter->isAbsolute());
	}
	
	for (std::list<URL>::iterator absIter = absURLs.begin(); absIter != absURLs.end(); absIter++) {
		for (std::list<URL>::iterator relIter = relURLs.begin(); relIter != relURLs.end(); relIter++) {
			URL relURL(*relIter);
			relURL.toAbsolute(*absIter);
			assert(relURL.isAbsolute());
		}
	}

}

int main(int argc, char** argv) {
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	// some URLs from http://www-archive.mozilla.org/quality/networking/testing/filetests.html

	HTTPServer::getInstance(8099, 8100);

	std::string exeName = argv[0];
	exeName = exeName.substr(exeName.find_last_of("\\/") + 1);

	testFileURLs();

	{
		try {
			URL url("http://asdfasdfasdfasdf.wgferg");
			url.download(true);
			assert(false);
		} catch (Event e) {
		}
	}

	{
		try {
			URL url("http://uscxml.tk.informatik.tu-darmstadt.de/foobarfoo.html");
			url.download(true);
			assert(false);
		} catch (Event e) {
			std::cout << e << std::endl;
		}
	}

#if 0
	{
		Interpreter interpreter = Interpreter::fromURI("/Users/sradomski/Desktop/application_small.scxml");
		assert(interpreter);
		std::list<std::string> states;
		states.push_back("b");
		interpreter.setConfiguration(states);
		interpreter.interpret();
	}
#endif

	{
		try {

			URL url(argv[0]);
			assert(canResolve(argv[0]));
			assert(canResolve(url.asString()));

			URL baseUrl = URL::asBaseURL(url);
			URL exeUrl(exeName);
			exeUrl.toAbsolute(baseUrl);
			assert(canResolve(exeUrl.asString()));
			std::cout << exeUrl.asString() << std::endl;
			exeUrl.download(true);
			assert(exeUrl.getInContent().length() > 0);

		} catch (Event e) {
			std::cout << e << std::endl;
		}
	}

	{
		TestServlet* testServlet1 = new TestServlet(false);
		TestServlet* testServlet2 = new TestServlet(false);

		assert(HTTPServer::registerServlet("/foo", testServlet1));
		assert(!HTTPServer::registerServlet("/foo", testServlet2));
		HTTPServer::unregisterServlet(testServlet1);
		assert(HTTPServer::registerServlet("/foo", testServlet2));
		HTTPServer::unregisterServlet(testServlet1);

		assert(HTTPServer::registerServlet("/foo/bar/", testServlet1));
		assert(!HTTPServer::registerServlet("/foo/bar/", testServlet2));
		HTTPServer::unregisterServlet(testServlet1);
		HTTPServer::unregisterServlet(testServlet2);
	}

	{
		TestServlet* testServlet1 = new TestServlet(true);
		TestServlet* testServlet2 = new TestServlet(true);
		TestServlet* testServlet3 = new TestServlet(true);

		assert(HTTPServer::registerServlet("/foo", testServlet1));
		assert(HTTPServer::registerServlet("/foo", testServlet2));
		assert(HTTPServer::registerServlet("/foo", testServlet3));
		assert(boost::ends_with(testServlet1->_actualUrl, "foo"));
		assert(boost::ends_with(testServlet2->_actualUrl, "foo2"));
		assert(boost::ends_with(testServlet3->_actualUrl, "foo3"));

		HTTPServer::unregisterServlet(testServlet1);
		HTTPServer::unregisterServlet(testServlet2);
		HTTPServer::unregisterServlet(testServlet3);
	}

	{
		Data data = Data::fromJSON("{\"shiftKey\":false,\"toElement\":{\"id\":\"\",\"localName\":\"body\"},\"clientY\":38,\"y\":38,\"x\":66,\"ctrlKey\":false,\"relatedTarget\":{\"id\":\"\",\"localName\":\"body\"},\"clientX\":66,\"screenY\":288,\"metaKey\":false,\"offsetX\":58,\"altKey\":false,\"offsetY\":30,\"fromElement\":{\"id\":\"foo\",\"localName\":\"div\"},\"screenX\":-1691,\"dataTransfer\":null,\"button\":0,\"pageY\":38,\"layerY\":38,\"pageX\":66,\"charCode\":0,\"which\":0,\"keyCode\":0,\"detail\":0,\"layerX\":66,\"returnValue\":true,\"timeStamp\":1371223991895,\"eventPhase\":2,\"target\":{\"id\":\"foo\",\"localName\":\"div\"},\"defaultPrevented\":false,\"srcElement\":{\"id\":\"foo\",\"localName\":\"div\"},\"type\":\"mouseout\",\"cancelable\":true,\"currentTarget\":{\"id\":\"foo\",\"localName\":\"div\"},\"bubbles\":true,\"cancelBubble\":false}");
		std::cout << data << std::endl;
	}

	{
		Data data = Data::fromJSON("asdf");
		std::cout << data << std::endl;
	}
	{
		Data data = Data::fromJSON("[ '1', '2', '3', '4' ]");
		std::cout << data << std::endl;
	}
	{
		Data data = Data::fromJSON("{'foo1': 'bar2', 'foo3': { 'foo4': 'bar5' }, 'foo6': 'bar7',  'foo8': { 'foo9': 'foo10': { 'foo11': 'bar12' } } }");
		std::cout << data << std::endl;
	}
	{
		Data data = Data::fromJSON("{\"firstName\": \"John\", \"lastName\": \"Smith\", \"age\": 25, \"address\": { \"streetAddress\": \"21 2nd Street\", \"city\": \"New York\",\"state\": \"NY\",\"postalCode\": 10021},\"phoneNumber\": [{\"type\": \"home\",\"number\": \"212 555-1234\"},{ \"type\": \"fax\",\"number\": \"646 555-4567\"}]}");
		std::cout << data << std::endl;
	}

	{
		URL url("http://www.heise.de/index.html");
		std::cout << url.asString() << std::endl;
		assert(url.isAbsolute());
		assert(iequals(url.scheme(), "http"));
		assert(iequals(url.host(), "www.heise.de"));
		assert(iequals(url.port(), "80"));
		assert(iequals(url.path(), "/index.html"));
		assert(iequals(url.asString(), "http://www.heise.de/index.html"));
		std::stringstream content;
		content << url;
	}

#ifndef _WIN32
	{
		URL url("https://raw.github.com/tklab-tud/uscxml/master/test/samples/uscxml/test-ecmascript.scxml");
		std::cout << url.asString() << std::endl;
		assert(url.isAbsolute());
		assert(iequals(url.scheme(), "https"));
		std::stringstream content;
		content << url;
	}
#endif
	
	{
		URL url("test/index.html");
		assert(iequals(url.scheme(), ""));
		url.toAbsoluteCwd();
		assert(iequals(url.scheme(), "file"));
		std::cout << url.asString() << std::endl;
	}

	{
		URL url = URL::toLocalFile("this is quite some content!", "txt");
		std::cout << url.asLocalFile("txt");
		assert(url.isAbsolute());
		assert(iequals(url.scheme(), "file"));
	}

}