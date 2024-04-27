Based on Swift's Networking Foundation

class URLSession {
	URLSession defaultInstance;

	URLSession();
	// URLSession(URLSessionConfiguration sessionWithConfiguration);

	// URLSessionConfiguration configuration; -> donnerait les configs de base des tasks sans URLRequest mais juste URL

	(URLSessionDataTask *) dataTaskWithURL(URL url, (void (^)(Data *data, URLResponse *response, Error *error)), completionHandler);
	(URLSessionDataTask *) dataTaskWithRequest(URLRequest request, (void (^)(Data *data, URLResponse *response, Error *error)), completionHandler);


	(URLSessionDownloadTask *) downloadTaskWithURL(URL url, (void (^)(URL location, URLResponse *response, Error *error)) completionHandler);
	(URLSessionDownloadTask *) downloadTaskWithRequest(URLRequest request, (void (^)(URL *location, URLResponse *response, Error *error)) completionHandler);


	(URLSessionUploadTask *) uploadTaskWithRequest(URLRequest request, Data *bodyData, (void (^)(Data *data, URLResponse *response, Error *error)) completionHandler);
	(URLSessionUploadTask *) uploadTaskFromFileWithRequest(URLRequest request, URL fileURL, (void (^)(Data *data, URLResponse *response, Error *error)) completionHandler);


	(URLSessionWebSocketTask *) webSocketTaskWithURL(URL url); // avec des closure delegate en plus
	(URLSessionWebSocketTask *) webSocketTaskWithRequest(URLRequest request); // avec des closure delegate en plus

	vector<URLSessionTask> getAllTasks();
}

struct URLRequest {
	URLRequest(URL url, HTTPMethod httpMethod = GET, Data* httpBody = nullptr) // et le reste peut être set avec des options

	HTTPMethod httpMethod;

	URL url;

	Data* httpBody;

	map<String, String> allHTTPHeaderFields;

	uint64_t timeoutInterval;

	enum HTTPMethod {
		GET, POST, PUT, DELETE
	}
}

virtual class URLSessionTask {
	void cancel();

	void resume();

	void suspend();

	State state;

	double progress; // y'aura aussi des callbacks pour observer ça

	uint64_t countOfBytesReceived;

	uint64_t countOfBytesExpectedToSend;

	enum State {
		running, suspended, cancelling, completed
	}
}

Pour ce qui est des URLSessionDataTask, URLSessionDownloadTask, URLSessionUploadTask et URLSessionWebSocketTask qui sont toutes dérivées de URLSessionTask, elles ont des callbacks en plus de base en fonction de ce qu'elles font.






