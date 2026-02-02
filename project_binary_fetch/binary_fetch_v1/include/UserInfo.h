#pragma once
#include <string>
#include <iostream>
using namespace std;

class UserInfo
{
public:
	string get_username();
	string get_computer_name();
	string get_domain_name();
	string get_user_groups();

};