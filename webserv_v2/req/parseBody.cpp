#include "HTTPRequest.hpp"

std::map<std::string, std::string> parseHeaders(std::string Header)
{
	std::map<std::string, std::string> headers;
	size_t pos = Header.find(":");
	if (pos != std::string::npos)
	{
		std::string key = Header.substr(0, pos);
		std::string value = Header.substr(pos + 1);
		trim(key);
		trim(value);
		headers[key] = value;
	}
	return headers;
}

std::map<std::string, std::string> parseArg(std::string arg)
{
	std::map<std::string, std::string> args;
	size_t pos = arg.find("=");
	if (pos != std::string::npos)
	{
		std::string key = arg.substr(0, pos);
		std::string value = arg.substr(pos + 1);
		trim(key);
		trim(value);
		if (value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"')
			value = value.substr(1, value.size() - 2);
		args[key] = value;
	}
	return args;
}

void HTTPRequest::parseFirstLine(const std::string &line)
{
	BodyPart part;
	size_t pos = line.find(";");
	if (pos != std::string::npos)
	{
		std::string Header = line.substr(0, pos);
		part.headers = parseHeaders(Header);
		std::string key = line.substr(pos + 1);
		key += ";";
		std::vector<std::string> args;
		size_t nextPos = 0;
		while ((pos = key.find(";", nextPos)) != std::string::npos)
		{
			args.push_back(key.substr(nextPos, pos - nextPos));
			nextPos = pos + 1;
		}
		for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); ++it)
		{
			std::map<std::string, std::string> arg = parseArg(*it);
			if (arg.find("filename") != arg.end())
				part.files = arg;
			else
				part.data = arg;
		}
	}
	else
		part.headers = parseHeaders(line);
	this->body_parts.push_back(part);
}

void HTTPRequest::parseURLEncodedBody(std::string body)
{
	std::vector<std::map<std::string, std::string> > data;
    std::istringstream iss(body);
    std::string key_value;
    while (std::getline(iss, key_value, '&'))
    {
        trim(key_value);
        size_t pos = key_value.find('=');
        std::map<std::string, std::string> key_value_map;
        if (pos != std::string::npos)
        {
            std::string key = key_value.substr(0, pos);
            std::string value = key_value.substr(pos + 1);
            trim(key);
            trim(value);
            if (value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"')
                value = value.substr(1, value.size() - 2);
            key_value_map[urlDecode(key)] = urlDecode(value);
        }
        else
            key_value_map[urlDecode(key_value)] = "";
        data.push_back(key_value_map);
    }
	for (std::vector<std::map<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
	{
		BodyPart part;
    	part.data = *it;
		this->body_parts.push_back(part);
	}
}

bool HTTPRequest::parseBody(std::istringstream &iss)
{
	std::string line;
	std::string body;
	while (std::getline(iss, line))
		body += line + "\n";
	trim(body);
	std::string contentType = getHeader("content-type");
	size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos)
	{
		if (contentType == "application/x-www-form-urlencoded")
			return (parseURLEncodedBody(body), true);
		std::cout << "Boundary not found in Content-Type header" << std::endl;
		return true;
	}
	std::string boundary = "--" + contentType.substr(boundaryPos + 9);
	trim(boundary);
	std::vector<std::string> parts;
	size_t pos = 0, nextPos = 0;
	while ((pos = body.find(boundary, nextPos)) != std::string::npos)
	{
		nextPos = pos + boundary.length();
		if (body.compare(nextPos, 2, "--") == 0)
			break;
		nextPos += 2;
		size_t endPos = body.find(boundary, nextPos);
		if (endPos == std::string::npos)
			endPos = body.size();
		std::string part = body.substr(nextPos, endPos - nextPos);
		trim(part);
		if (!part.empty())
			parts.push_back(part);
		nextPos = endPos;
	}
	for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it)
	{
		std::istringstream partStream(*it);
		std::string FirstLine;
		std::getline(partStream, FirstLine);
		trim(FirstLine);
		parseFirstLine(FirstLine);
		std::string line;
		std::string data_block;
		while (std::getline(partStream, line))
		{
			trim(line);
			if (line.empty())
				continue;
			size_t pos = line.find("Content-Type:");
			if (pos != std::string::npos)
			{
				std::string Header = line.substr(pos + 13);
				trim(Header);
				this->body_parts.back().content_type = Header;
				continue;
			}
			data_block += line + "\n";
		}
		if (!data_block.empty())
		{
			trim(data_block);
			if(this->body_parts.back().files.empty())
			{
				std::map<std::string, std::string> data = this->body_parts.back().data;
				std::map<std::string, std::string> newData;
				for (std::map<std::string, std::string>::iterator it2 = data.begin(); it2 != data.end(); ++it2)
					newData[it2->second] = data_block;
				this->body_parts.back().data = newData;
			}
			else if(!(this->body_parts.back().files.empty()))
			{
				std::map<std::string, std::string> files = this->body_parts.back().files;
				std::map<std::string, std::string> newFiles;
				for (std::map<std::string, std::string>::iterator it2 = files.begin(); it2 != files.end(); ++it2)
					newFiles[it2->second] = data_block;
				this->body_parts.back().files = newFiles;
			}
		}
	}
	return true;
}