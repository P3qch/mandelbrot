#pragma once
#include <stdexcept>

class SdlException : public std::exception
{
public:
	using std::exception::exception;
};

class AppException : public std::exception
{
public:
	using std::exception::exception;
};

class QuitException : public AppException
{
public:
	using AppException::AppException;
};