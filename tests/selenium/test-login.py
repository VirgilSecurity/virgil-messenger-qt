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

print("Click Login button")
wd.find_element_by_xpath("//OutlineButton[@objectName='bLogin' and @visible='true']").click()

print("Enter user name")
wd.find_element_by_xpath("//TextField[@objectName='tfLoginUsername' and @visible='true']").clear()
wd.find_element_by_xpath("//TextField[@objectName='tfLoginUsername' and @visible='true']").send_keys(userID)

print("Click Sign In button")
sleep(1)
wd.find_element_by_xpath("//PrimaryButton[@objectName='bSignIn' and @visible='true']").click()

print("Sleep 10s and save a Screen Shot")
sleep(10)
save_screenshot('login-button-pressed')

print("Check is correctly Logged In")
try:
    wd.find_element_by_xpath("//Button[@objectName='bSendMsg' and @visible='true']")
    print("Log In: SUCCESS")
except:
    print("Log In: FAIL")

print("Stop")
#wd.close()