#!/usr/bin/python
import sys
from time import sleep
from selenium import webdriver
from optparse import OptionParser
from datetime import datetime
import time

# ****************************************************************************
# Initialization parameters
usage = "usage: %prog "
parser = OptionParser(usage=usage)
parser.add_option("-u", "--user", dest="user", help="User name", metavar="USER")
parser.add_option("-c", "--connect", dest="connect", help="Connect string", metavar="CONNECT")
parser.add_option("-r", "--r", dest="respath", help="Log and screenshot path", metavar="RESPATH")
(options, args) = parser.parse_args()

# Check parameters
if not options.user:
    userID = "selenium%s" % time.mktime(datetime.now().timetuple())
else:
    userID = options.user

if not options.connect:
    WD_ADDRESS = 'http://localhost:9517'
else:
    WD_ADDRESS = options.connect

if not options.respath:
    RESPATH = "/home/jenkins/result/screenshot"
else:
    RESPATH = options.respath

# ****************************************************************************    
print("User ID is ", userID)
print("Connecting [%s]..." % WD_ADDRESS)
wd = webdriver.Remote(WD_ADDRESS,
                      desired_capabilities={'browserStartWindow': '*'})


# ****************************************************************************
def save_screenshot(name):
    print("Save screenshot to %s/%s.png" % (RESPATH, name))
    wd.get_screenshot_as_file("%s/%s.png" % (RESPATH, name))

# ****************************************************************************
sleep(4)

print("Click Register button")
wd.find_element_by_xpath("//FormPrimaryButton[@objectName='btnRegister' and @visible='true']").click()

print("Enter user name")
wd.find_element_by_xpath("//TextField[@visible='true']").clear()
wd.find_element_by_xpath("//TextField[@visible='true']").send_keys(userID)

print("Click \"Create account\" button")
sleep(1)
wd.find_element_by_xpath("//FormPrimaryButton[@objectName='btnCreateAccount' and @visible='true']").click()

print("Sleep 10s and save a Screen Shot")
sleep(10)
save_screenshot('register-button-pressed')

print("Check is correctly Registered")
try:
    wd.find_element_by_xpath("//ContactsHeader[@objectName='hdrDefaultServer' and @visible='true']")
    print("Log In: SUCCESS")
except:
    print("Log In: FAIL")

print("Stop")
wd.close()