#include "multipart_parser.h"

namespace nicer {
namespace utils {

/**
* Implmentation of class HeaderParser.
*
* Locates and extracts MIME headers, and stores their field keynames and values.
*/

HeaderParser::HeaderParser(Object *obj) {
    this->obj = obj;
    state = PREKEY;
}

/**
* @param string input -- string to split.
* @param string left -- output left half of string after split.
* @param string right -- output right half of string after split.
* @param char spl -- split on this character, include in left half output.
*/
void HeaderParser::split(std::string &input, std::string &left, std::string &right, unsigned char spl) {
    int pos = input.find(spl);

    if (pos >= 0) {
        right = input.substr(pos + 1);
        left = input.substr(0, pos);
    } else {
        left = input;
        right = "";
    }
}


void HeaderParser::parse_attrs(HeaderField &fld) {
    std::string a, b;

    while (*(fld.key.begin()) == ' ')
        fld.key = fld.key.substr(1);
    while (fld.key[fld.key.size() - 1] == ' ')
        fld.key.erase(fld.key.end() - 1);

    while (*(fld.value.begin()) == ' ')
        fld.value = fld.value.substr(1);
    while (fld.value[fld.value.size() - 1] == ' ')
        fld.value.erase(fld.value.end() - 1);

    split(fld.value, a, b, ';');

    fld.value = a;

    a = b;

    while (a != "") {
        std::string c, d;

        split(a, a, b, ';');

        split(a, c, d, '=');

        while (*(c.begin()) == ' ')
            c = c.substr(1);
        while (c[c.size() - 1] == ' ')
            c.erase(c.end() - 1);

        while (*(d.begin()) == ' ')
            d = d.substr(1);
        while (d[d.size() - 1] == ' ')
            d.erase(d.end() - 1);

        while (*(d.begin()) == '"')
            d = d.substr(1);
        while (d[d.size() - 1] == '"')
            d.erase(d.end() - 1);

        fld.attributes[c] = d;

        a = b;
    }
}

void HeaderParser::add_header(const std::string &key, const std::string &value) {
    HeaderField fld;

    fld.key = key;
    fld.value = value;

    if (key == "content-type")
        parse_attrs(fld);

    // CLx
    if (key == "content-disposition")
        parse_attrs(fld);

    obj->fields[key] = fld;
}


void HeaderParser::parse(unsigned char c) {
    switch (state) {
        case CR:
            if (c == '\n') {
                state = EOL;
                return;
            }
            throw error("CR without LF in MIME header?");

        case EOL:

            // This deals with continuation.
            if (c == ' ' || c == '\t') {
                state = VAL;
                return;
            }

            // Not a continue, save key/value if there is one.
            if (key != "") {
                add_header(key, value);
            }

            if (c == '\r') return;

            if (c == '\n') {

                if (obj->fields.find("content-type") != obj->fields.end()) {
                    std::string ct = obj->fields["content-type"].value;
                    if (std::string::npos != ct.find('/')) {
                        obj->type = ct.substr(0, ct.find('/'));
                        obj->subtype = ct.substr(ct.find('/') + 1);
                    }
                }

                throw end_of_header("EOH");
            }

            // EOL, now reading a key.

            state = KEY;
            key = tolower(c);
            return;

        case PREKEY:
            if (c == ' ' || c == '\t') return;
            if (c == ':') throw error("Zero length key?");
            state = KEY;
            key = tolower(c);
            return;

        case KEY:
            if (c == ' ' || c == '\t') return;
            if (c == ':') {
                state = PREVAL;
                return;
            }
            key += tolower(c);
            return;

        case PREVAL:
            if (c == ' ' || c == '\t') return;
            if (c == '\r') {
                state = CR;
                return;
            }
            if (c == '\n') {
                state = EOL;
                return;
            }
            value = c;
            state = VAL;
            return;

        case VAL:
            if (c == '\r') {
                state = CR;
                return;
            }
            if (c == '\n') {
                state = EOL;
                return;
            }

            value += c;
            return;

        case ATTRKEY:
        case ATTRVAL:
        case POSTVAL:
        case QTVAL:
            // TODO: NOT IMPLEMENTED, do we throw or just return?
            // throw error("Unimplemented header feature");

            return;
    }
}

// Constructor
AnyObjectParser::AnyObjectParser(ClientInterface *client, Object *obj) : ObjectParser(client, obj) {
    p = new HeaderParser(obj);
    state = HEADER;
}

void AnyObjectParser::parse(unsigned char c) {
    if (state == HEADER_COMPLETE) {
        delete p;

        state = BODY;
        client->object_created(obj);

        // Use the parser factory to get an appropriate parser.
        p = ParserFactory::create(client, obj);
    }

    try {
        p->parse(c);
    } catch (end_of_header &e) {
        state = HEADER_COMPLETE;
    }
}

void AnyObjectParser::close() {
    if (p) {
        p->close();
        delete p;
    }
}

ObjectParser::ObjectParser(ClientInterface *client, Object *obj) {
    this->client = client;
    this->obj = obj;
}

// Constructor
MultipartParser::MultipartParser(ClientInterface *client, Object *obj) : ObjectBodyParser(client, obj) {
    state = PRE;
    posn = 0;
    boundary = "--" + obj->fields["content-type"].attributes["boundary"];
    sub_obj = 0;
    sub_parser = 0;
}


void MultipartParser::parse(unsigned char c) {
    if (state == ATBOUND) {
        if (c == '-') {
            state = FOLLBOUND;
            return;
        }
        if (c == '\r') {
            state = FOLLBOUND;
            return;
        }
        if (c == '\n') {
            state = STARTOBJECT;
            return;
        }
        throw error("MIME duff boundary stuff");
    }

    if (state == FOLLBOUND) {
        if (c == '-') {
            state = DONE;
            return;
        }
        if (c == '\n') {
            state = STARTOBJECT;
            return;
        }
        throw error("MIME duff boundary stuff");
    }

    if (state == PRE) {

        if (c == boundary[posn])
            posn++;
        else {
            if (c == boundary[0])
                posn = 1;
            else
                posn = 0;
        }

        if (posn == boundary.size()) {
            state = ATBOUND;
            posn = 0;
            return;
        }

        return;
    }

    if (state == STARTOBJECT) {

        if (sub_parser) {
            sub_parser->close();
            delete sub_parser;
        }

        if (sub_obj) {
            delete sub_obj;
            sub_obj = 0;
        }

        sub_obj = new Object();
        sub_obj->parent = ObjectParser::obj;

        sub_parser = ParserFactory::generic(ObjectParser::client, sub_obj);

        state = INSIDE;

        sub_parser->parse(c);

        posn = 0;
        buffer = "";

        return;
    }

    if (state == INSIDE) {
        if (c == boundary[posn]) {
            posn++;
            buffer += c;
        } else {
            // Dispose of buffer
            if (posn) {
                for (size_t i = 0; i < buffer.size(); i++)
                    sub_parser->parse(buffer[i]);
                buffer = "";
                posn = 0;
            }

            // This character may start the match again.
            if (c == boundary[0]) {
                posn = 1;
                buffer += c;
            } else {
                posn = 0;
                sub_parser->parse(c);
            }
        }

        if (posn == boundary.size()) {
            state = ATBOUND;
            posn = 0;
            return;
        }

        return;
    }
}

void MultipartParser::close() {
    // Dispose of buffer
    if (posn) {
        for (size_t i = 0; i < buffer.size(); i++)
            sub_parser->parse(buffer[i]);
        buffer = "";
    }

    if (sub_parser) {
        sub_parser->close();
        delete sub_parser;
    }

    if (sub_obj) {
        delete sub_obj;
        sub_obj = 0;
    }
}

Parser *ParserFactory::generic(ClientInterface *client, Object *obj) {
    return new AnyObjectParser(client, obj);
}

// current only multipart supported
Parser *ParserFactory::create(ClientInterface *client, Object *obj) {
    std::string encoding =
            obj->fields["content-transfer-encoding"].value;

    return new MultipartParser(client, obj);
}

/**
 * Default constructor.
 */
Decoder::Decoder() {
    parser = 0;
    obj = new Object();
}

void Decoder::decode(unsigned char c)
{
    if (!parser) {
        parser = ParserFactory::generic(this, obj);
    }
    parser->parse(c);
}

/**
 * The data producer should call this once all data has been presented
 * to the decoder to finalise processing and clean up all resources.
 */
void Decoder::close()
{
    if (parser) {
        parser->close();
        delete parser;
    }
    delete obj;
}

/**
 * Implementation of class StringDecoder.
 *
 * Decodes a MIME string and allows retrieving form data values given a key when MIME
 * content type is multipart/form-data.
 *
 * @author Chris Johnson
 */

/**
 * Called when a MIME object is discovered in the input stream.  Data may or may not follow.
 * @param object - an instance representing a MIME object.
 */
/* protected */
void StringDecoder::object_created(Object *object) {
}

/**
 * Called when a MIME object's data starts to be read.
 * @param object - an instance representing a MIME object.
 */
/* protected */
void StringDecoder::data_start(Object *object) {
    // every time new data starts, initialize a new part string.
    part = "";
}

/**
 * Called once per byte in the MIME object's data.  Saves that data to
 * our class string for later use.
 * @param data - pointer to actual data.
 * @param len - number of bytes in data passed.
 * @param object - an instance representing a MIME object.
 */
/* protected */
void StringDecoder::data(Object *object, unsigned char *data, int len) {
    for (int i = 0; i < len; i++) {
        // for each byte in the data stream, append it to MIME part string.
        part += data[i];
    }
}

/**
 * Called when all the data from the MIME object has been delivered.  Saves
 * the string we built and the key name (e.g. form item name) to our hash (map)
 * for future access.
 * @param object - an instance representing a MIME object.
 */
/* protected */
void StringDecoder::data_end(Object *object) {
    HeaderField hf = object->fields.at("content-disposition");
    std::string name = hf.attributes.at("name");

    // End of data stream aka MIME part, save form NAME and data VALUE.
    results[name] = part;
}

/**
 * Public API method specifies MIME string to decode.
 * @param string - the MIME string to decode.
 */
void StringDecoder::processString(std::string input) {
    for ( std::string::iterator it=input.begin(); it!=input.end(); ++it) {
        decode(*it);
    }
}

/**
 * Public API method retrieves a MIME form data element value.
 * @param string - the MIME form data element name to retrieve.
 * @return string - the form data.
 */
std::string StringDecoder::getFormValue(std::string formName) {
    if (results.count(formName) > 0) {
        return results.at(formName);
    } else {
        return "";
    }
}

/**
 * Public API method dumps all known MIME form data key-value pairs.
 * @return string - one line per key-value pair.
 */
std::string StringDecoder::dump() {
    std::string ret = "Form key-value dump:\n";
    std::map <std::string, std::string>::iterator it;
    for (it = results.begin(); it != results.end(); it++) {
        ret += "results[" + it->first + "] = " + it->second + '\n';
    }
    return ret;
}

}   // namespace utils
}   // namespace nicer