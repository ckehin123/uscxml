/**
 *  @file
 *  @author     2012-2013 Stefan Radomski (stefan.radomski@cs.tu-darmstadt.de)
 *  @copyright  Simplified BSD
 *
 *  @cond
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the FreeBSD license as published by the FreeBSD
 *  project.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  You should have received a copy of the FreeBSD license along with this
 *  program. If not, see <http://www.opensource.org/licenses/bsd-license>.
 *  @endcond
 */

#include "JSCDocument.h"
#include "JSCElement.h"
#include "JSCAttr.h"
#include "JSCStorage.h"
#include "JSCXPathResult.h"
#include "JSCNode.h"
#include <XPath/XPath.hpp>
#include <DOM/io/Stream.hpp>

namespace Arabica {
namespace DOM {

JSValueRef JSCDocument::localStorageCustomAttrGetter(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception) {
	struct JSCDocumentPrivate* privData = (struct JSCDocumentPrivate*)JSObjectGetPrivate(object);

	if (!privData->dom->storage) {
		return JSValueMakeUndefined(ctx);
	}

	JSClassRef retClass = JSCStorage::getTmpl();
	struct JSCStorage::JSCStoragePrivate* retPrivData = new JSCStorage::JSCStoragePrivate();
	retPrivData->dom = privData->dom;
	retPrivData->nativeObj = retPrivData->dom->storage;

	JSObjectRef arbaicaRetObj = JSObjectMake(ctx, retClass, retPrivData);
	return arbaicaRetObj;

}


JSValueRef JSCDocument::evaluateCustomCallback(JSContextRef ctx, JSObjectRef function, JSObjectRef object, size_t argumentCount, const JSValueRef* arguments, JSValueRef* exception) {
	struct JSCDocumentPrivate* privData = (struct JSCDocumentPrivate*)JSObjectGetPrivate(object);

	if (!privData->dom || !privData->dom->xpath) return JSValueMakeUndefined(ctx);
	if (argumentCount < 1) {
		std::string errorMsg = "Wrong number of arguments in evaluate";
		JSStringRef string = JSStringCreateWithUTF8CString(errorMsg.c_str());
		JSValueRef exceptionString = JSValueMakeString(ctx, string);
		JSStringRelease(string);
		*exception = JSValueToObject(ctx, exceptionString, NULL);
		return JSValueMakeUndefined(ctx);
	}

	// make sure first argument is a string
	if (!JSValueIsString(ctx, arguments[0])) {
		std::string errorMsg = "Expected xpath expression as first argument";
		JSStringRef string = JSStringCreateWithUTF8CString(errorMsg.c_str());
		JSValueRef exceptionString = JSValueMakeString(ctx, string);
		JSStringRelease(string);
		*exception = JSValueToObject(ctx, exceptionString, NULL);
		return JSValueMakeUndefined(ctx);
	}

	JSStringRef stringReflocalXPath = JSValueToStringCopy(ctx, arguments[0], NULL);
	size_t localXPathMaxSize = JSStringGetMaximumUTF8CStringSize(stringReflocalXPath);
	char* localXPathBuffer = new char[localXPathMaxSize];
	JSStringGetUTF8CString(stringReflocalXPath, localXPathBuffer, localXPathMaxSize);
	std::string localXPath(localXPathBuffer);
	JSStringRelease(stringReflocalXPath);
	free(localXPathBuffer);

	JSClassRef arbaicaRetClass = JSCXPathResult::getTmpl();

	XPath::XPathValue<std::string>* retVal;

	try {
		if (argumentCount > 1) {
			// make sure second argument is a node
			if (!JSValueIsObject(ctx, arguments[1])) {
				std::string errorMsg = "Second argument is not of type node";
				JSStringRef string = JSStringCreateWithUTF8CString(errorMsg.c_str());
				JSValueRef exceptionString = JSValueMakeString(ctx, string);
				JSStringRelease(string);
				*exception = JSValueToObject(ctx, exceptionString, NULL);
				return JSValueMakeUndefined(ctx);
			}

//			Arabica::DOM::Node<std::string>* localContextNode = (Arabica::DOM::Node<std::string>*)JSObjectGetPrivate(JSValueToObject(ctx, arguments[1], NULL));
			JSCNode::JSCNodePrivate* otherNodePrivate = (JSCNode::JSCNodePrivate*)JSObjectGetPrivate(JSValueToObject(ctx, arguments[1], NULL));
			Arabica::DOM::Node<std::string>* localContextNode = otherNodePrivate->nativeObj;

//			std::cout << *localContextNode << std::endl;
//			std::cout << ">>" << privData->dom->xpath->evaluate("//note/@importance", *localContextNode).asString() << "<<" << std::endl;

			retVal = new XPath::XPathValue<std::string>(privData->dom->xpath->evaluate(localXPath, *localContextNode));
		} else {
			retVal = new XPath::XPathValue<std::string>(privData->dom->xpath->evaluate(localXPath, *privData->nativeObj));
		}
	} catch (std::runtime_error e) {
		std::cout << e.what() << std::endl;
		return JSValueMakeUndefined(ctx);
	}

	struct JSCXPathResult::JSCXPathResultPrivate* retPrivData = new JSCXPathResult::JSCXPathResultPrivate();
	retPrivData->dom = privData->dom;
	retPrivData->nativeObj = retVal;

	JSObjectRef arbaicaRetObj = JSObjectMake(ctx, arbaicaRetClass, retPrivData);
	return arbaicaRetObj;

#if 0
	if (args.Length() < 1)
		throw V8Exception("Wrong number of arguments in evaluate");
//  if (!((V8Node::hasInstance(args[1])) && (V8XPathResult::hasInstance(args[3]))))
//    throw V8Exception("Parameter mismatch while calling evaluate");

	v8::Local<v8::Object> self = args.Holder();
	V8DocumentPrivate* privData = V8DOM::toClassPtr<V8DocumentPrivate >(self->GetInternalField(0));

	v8::String::AsciiValue localExpression(args[0]);

	XPath::XPathValue<std::string>* retVal;
	if (args.Length() > 1) {
		Arabica::DOM::Node<std::string>* localContextNode = V8DOM::toClassPtr<Arabica::DOM::Node<std::string> >(args[1]->ToObject()->GetInternalField(0));
		retVal = new XPath::XPathValue<std::string>(privData->dom->xpath->evaluate(*localExpression, *localContextNode));
	} else {
		retVal = new XPath::XPathValue<std::string>(privData->dom->xpath->evaluate(*localExpression, *privData->nativeObj));
	}

	v8::Handle<v8::Function> retCtor = V8XPathResult::getTmpl()->GetFunction();
	v8::Persistent<v8::Object> retObj = v8::Persistent<v8::Object>::New(retCtor->NewInstance());

	V8XPathResult::V8XPathResultPrivate* retPrivData = new V8XPathResult::V8XPathResultPrivate();
	retPrivData->dom = privData->dom;
	retPrivData->nativeObj = retVal;

	retObj->SetInternalField(0, V8DOM::toExternal(retPrivData));

	retObj.MakeWeak(0, V8XPathResult::jsDestructor);
	return retObj;
#endif
}

JSValueRef JSCDocument::createElementNSCustomCallback(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObj, size_t argumentCount, const JSValueRef* arguments, JSValueRef* exception) {

	struct JSCDocumentPrivate* privData = (struct JSCDocumentPrivate*)JSObjectGetPrivate(thisObj);

	if (false) {
	} else if (argumentCount == 2 &&
	           JSValueIsString(ctx, arguments[0]) &&
	           JSValueIsString(ctx, arguments[1])) {
		JSStringRef stringReflocalNamespaceURI = JSValueToStringCopy(ctx, arguments[0], exception);
		size_t localNamespaceURIMaxSize = JSStringGetMaximumUTF8CStringSize(stringReflocalNamespaceURI);
		char* localNamespaceURIBuffer = new char[localNamespaceURIMaxSize];
		JSStringGetUTF8CString(stringReflocalNamespaceURI, localNamespaceURIBuffer, localNamespaceURIMaxSize);
		std::string localNamespaceURI(localNamespaceURIBuffer);
		JSStringRelease(stringReflocalNamespaceURI);
		free(localNamespaceURIBuffer);

		JSStringRef stringReflocalQualifiedName = JSValueToStringCopy(ctx, arguments[1], exception);
		size_t localQualifiedNameMaxSize = JSStringGetMaximumUTF8CStringSize(stringReflocalQualifiedName);
		char* localQualifiedNameBuffer = new char[localQualifiedNameMaxSize];
		JSStringGetUTF8CString(stringReflocalQualifiedName, localQualifiedNameBuffer, localQualifiedNameMaxSize);
		std::string localQualifiedName(localQualifiedNameBuffer);
		JSStringRelease(stringReflocalQualifiedName);
		free(localQualifiedNameBuffer);


		Arabica::DOM::Element<std::string>* retVal = new Arabica::DOM::Element<std::string>(privData->nativeObj->createElementNS(localNamespaceURI, localQualifiedName));
		if (privData->dom->nsInfo->nsToPrefix.find(localNamespaceURI) != privData->dom->nsInfo->nsToPrefix.end())
			retVal->setPrefix(privData->dom->nsInfo->nsToPrefix[localNamespaceURI]);

		JSClassRef retClass = JSCElement::getTmpl();

		struct JSCElement::JSCElementPrivate* retPrivData = new JSCElement::JSCElementPrivate();
		retPrivData->dom = privData->dom;
		retPrivData->nativeObj = retVal;

		JSObjectRef retObj = JSObjectMake(ctx, retClass, retPrivData);

		return retObj;

	}

	JSStringRef exceptionString = JSStringCreateWithUTF8CString("Parameter mismatch while calling createElementNS");
	*exception = JSValueMakeString(ctx, exceptionString);
	JSStringRelease(exceptionString);
	return JSValueMakeUndefined(ctx);
}

JSValueRef JSCDocument::createAttributeNSCustomCallback(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObj, size_t argumentCount, const JSValueRef* arguments, JSValueRef* exception) {

	struct JSCDocumentPrivate* privData = (struct JSCDocumentPrivate*)JSObjectGetPrivate(thisObj);

	if (false) {
	} else if (argumentCount == 2 &&
	           JSValueIsString(ctx, arguments[0]) &&
	           JSValueIsString(ctx, arguments[1])) {
		JSStringRef stringReflocalNamespaceURI = JSValueToStringCopy(ctx, arguments[0], exception);
		size_t localNamespaceURIMaxSize = JSStringGetMaximumUTF8CStringSize(stringReflocalNamespaceURI);
		char* localNamespaceURIBuffer = new char[localNamespaceURIMaxSize];
		JSStringGetUTF8CString(stringReflocalNamespaceURI, localNamespaceURIBuffer, localNamespaceURIMaxSize);
		std::string localNamespaceURI(localNamespaceURIBuffer);
		JSStringRelease(stringReflocalNamespaceURI);
		free(localNamespaceURIBuffer);

		JSStringRef stringReflocalQualifiedName = JSValueToStringCopy(ctx, arguments[1], exception);
		size_t localQualifiedNameMaxSize = JSStringGetMaximumUTF8CStringSize(stringReflocalQualifiedName);
		char* localQualifiedNameBuffer = new char[localQualifiedNameMaxSize];
		JSStringGetUTF8CString(stringReflocalQualifiedName, localQualifiedNameBuffer, localQualifiedNameMaxSize);
		std::string localQualifiedName(localQualifiedNameBuffer);
		JSStringRelease(stringReflocalQualifiedName);
		free(localQualifiedNameBuffer);


		Arabica::DOM::Attr<std::string>* retVal = new Arabica::DOM::Attr<std::string>(privData->nativeObj->createAttributeNS(localNamespaceURI, localQualifiedName));
		if (privData->dom->nsInfo->nsToPrefix.find(localNamespaceURI) != privData->dom->nsInfo->nsToPrefix.end())
			retVal->setPrefix(privData->dom->nsInfo->nsToPrefix[localNamespaceURI]);

		JSClassRef retClass = JSCAttr::getTmpl();

		struct JSCAttr::JSCAttrPrivate* retPrivData = new JSCAttr::JSCAttrPrivate();
		retPrivData->dom = privData->dom;
		retPrivData->nativeObj = retVal;

		JSObjectRef retObj = JSObjectMake(ctx, retClass, retPrivData);

		return retObj;

	}

	JSStringRef exceptionString = JSStringCreateWithUTF8CString("Parameter mismatch while calling createAttributeNS");
	*exception = JSValueMakeString(ctx, exceptionString);
	JSStringRelease(exceptionString);
	return JSValueMakeUndefined(ctx);
}

}
}