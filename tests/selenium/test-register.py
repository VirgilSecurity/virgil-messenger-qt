import sys
from time import sleep
from selenium import webdriver

if len(sys.argv) != 2:
    print("Set User ID as a parameter")
    exit(1)
    
userID = sys.argv[1]
print("User ID is ", userID)

WD_ADDRESS = 'http://localhost:9517'

print("Connecting ...")
wd = webdriver.Remote(WD_ADDRESS,
                      desired_capabilities={'browserStartWindow': '*'})

def save_screenshot(name):
    wd.get_screenshot_as_file('/tmp/wdscreenshots/%s.png' % name)


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
#wd.close()