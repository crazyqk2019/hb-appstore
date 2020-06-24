﻿#include "Feedback.hpp"
#include "ImageCache.hpp"
#include "MainDisplay.hpp"
#include "main.hpp"

#include "../libs/chesto/src/RootDisplay.hpp"

#ifndef NETWORK_MOCK
#include <curl/curl.h>
#include <curl/easy.h>
#endif

Feedback::Feedback(Package* package)
	: package(package)
	, title((std::string("发送反馈给：\"") + package->title + "\"").c_str(), 25)
	, icon(package->getIconUrl().c_str(), []{ return new ImageElement(RAMFS "res/default.png"); })
	, quit("取消", Y_BUTTON, true, 24)
	, send("提交", X_BUTTON, true, 24)
	, response("如果您想发送更多详细的反馈\n请发送邮件到我们的邮箱 fight@fortheusers.org", 20, NULL, false, 360)
#if defined(__WIIU__)
	, hint("(顺便提示：您可以按减号键退出！)", 20, NULL)
#endif
{
	title.position(50, 30);
	super::append(&title);

	icon.position(50, 160);
#if defined(_3DS) || defined(_3DS_MOCK)
  icon.resize(ICON_SIZE, ICON_SIZE);
#else
	icon.resize(256, ICON_SIZE);
#endif
	super::append(&icon);

	keyboard.inputCallback = std::bind(&Feedback::keyboardInputCallback, this);
	keyboard.x = 200;
	super::append(&keyboard);

	quit.position(470, 340);
	quit.action = std::bind(&Feedback::back, this);
	super::append(&quit);

	send.position(quit.x + quit.width + 25, quit.y);
	send.action = std::bind(&Feedback::submit, this);
	super::append(&send);

	response.position(860, 20);
	super::append(&response);

	feedback.setSize(23);
	feedback.setWrappedWidth(730);
	feedback.position(390, 140);
	super::append(&feedback);

#if defined(__WIIU__)
	hint.position(50, 120);
	super::append(&hint);
#endif
}

bool Feedback::process(InputEvents* event)
{
	bool ret = super::process(event);

	if (needsRefresh)
	{
		feedback.setText(keyboard.getTextInput());
		feedback.update();
		needsRefresh = false;
	}

	return ret;
}

void Feedback::keyboardInputCallback()
{
	needsRefresh = true;
}

void Feedback::submit()
{
#ifndef NETWORK_MOCK
	CURL* curl;
	CURLcode res;

#if defined(__WIIU__)
	const char* userKey = "wiiu_user";
#else
	const char* userKey = "switch_user";
#endif

	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://switchbru.com/appstore/feedback");
		std::string fields = std::string("name=") + userKey + "&package=" + package->pkg_name + "&message=" + keyboard.getTextInput();
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields.c_str());

		res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
#endif

	// close this window
	this->back();
}

void Feedback::back()
{
	RootDisplay::switchSubscreen(nullptr);
}
