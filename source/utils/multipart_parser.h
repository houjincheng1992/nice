#pragma once

#include <exception>
#include <map>
#include <stdexcept>
#include <string>

namespace nicer {
namespace  utils {

class error : public std::logic_error {
public:
	error(const std::string &msg) : std::logic_error(msg) {}
};

class end_of_header : public std::logic_error {
public:
	end_of_header(const std::string &msg) : std::logic_error(msg) {}
};

/**
 * Describes a MIME object header field.
 */
// TODO: this could probably be struct.
class HeaderField {
public:
	/** 
	 * The field key e.g. "content-type". Note that the key is converted to lower case.
	 */
	std::string key;

	/**
	 * The field value.
	 */
	std::string value;

	/**
	 * The attributes associated with the field.  This is a map from key to value.
	 */
	std::map<std::string, std::string> attributes;
};

/**
 * This object describes a MIME object but does not contain the data.
 */
// TODO: this probably could be just a struct.
class Object {
public:
	/**
	   The default constructor.
	*/
	Object() {
		parent = 0;
	}

	/**
	   The object's major type, derived from the MIME type
	   e.g. "text"
	*/
	std::string type;

	/**
	   The object's minor type, derived from the MIME type
	   e.g. "plain"
	*/
	std::string subtype;

	/**
	   All fields from the object's header
	*/
	std::map<std::string, HeaderField> fields;

	/**
	   Pointer to the parent object, if one exists.
	*/
	Object *parent;
};

/**
 * Interface describes the events called as a MIME string is parsed.
 * To make the MIME decoder to do something useful, implement all these methods.
 */
class ClientInterface {
public:
	/**
	 * A MIME object has been discovered in the input data.  Data may or may not follow.
	 */
	virtual void object_created(Object * object) = 0;

	/**
	 * A MIME object's data starts processing.
	 */
	virtual void data_start(Object * obj) = 0;

	/**
	 * Deliver data parsed from the MIME object.
	 */
	virtual void data(Object * obj, unsigned char *data, int len) = 0;

	/**
	 * All data from the MIME object has been delivered.
	 */
	virtual void data_end(Object * obj) = 0;
};

class Parser {
public:
	Parser() {}
	virtual ~Parser() {}

	virtual void parse(unsigned char c) = 0;
	virtual void close() {}
};

class ObjectParser : public Parser {
public:
	ObjectParser(ClientInterface *client, Object *obj);

	ClientInterface *client;
	Object *obj;
};

class ObjectBodyParser : public ObjectParser {
public:
	ObjectBodyParser(ClientInterface *client, Object *obj) : ObjectParser(client, obj) {
		// constructor does nothing other than initialize ObjectParser.
	}
};

class MultipartParser : public ObjectBodyParser {
private:
	Parser *sub_parser;
	Object *sub_obj;

	// States:
	// PRE - Searching for first boundary.
	// ATBOUND - Just matched the whole boundary string.
	// FOLLBOUND - Looking at characters after boundary.
	// INSIDE - Processing stuff between boundaries.
	// STARTOBJECT - Starting a MIME object.
	enum {
		PRE, ATBOUND, FOLLBOUND, INSIDE, DONE, STARTOBJECT
	} state;

	std::string boundary;
	std::string buffer;
	size_t posn;

public:
	MultipartParser(ClientInterface *client, Object *obj);

	virtual void parse(unsigned char c);

	void close();
};

class ParserFactory {
public:
	static Parser *generic(ClientInterface *client, Object *obj);

	static Parser *create(ClientInterface *client, Object *obj);
};

/**
 * A class which knows how to decode MIME.
 * To do something useful, derive a class from it, and implement
 * all the calls in the ClientInterface interface.
 */
class Decoder : public ClientInterface {
private:
	Parser *		parser;
	Object *		obj;
public:
 	Decoder();
	virtual ~Decoder() {};

	/**
	 * The data producer should call this once all data has been presented
	 * to the decoder to finalise processing and clean up all resources.
	 */
	void close();

	/**
	 * The data produced should call this method to present data to the
	 * decoder.
	 */
	void decode(unsigned char c);
};

class StringDecoder : public Decoder {
public:
	StringDecoder() {}
	~StringDecoder() {}

	void			processString(std::string input);
	std::string		getFormValue(std::string formName);
	std::string		dump();

protected:
	std::string			part;

	std::map<std::string, std::string>	results;

	virtual void object_created(Object *object);
	virtual void data_start(Object *obj);
	virtual void data(Object *obj, unsigned char *data, int len);
	virtual void data_end(Object *obj);
};


}	// namespace utils
}	// namespace nicer