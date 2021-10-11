import os
from typing import TypeVar, Generic

from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from webdriver_manager.chrome import ChromeDriverManager

T = TypeVar('T')


class SeleniumWebdriverError(Exception):
    pass


class SeleniumWebdriver(Generic[T]):

    def __init__(self):
        self.options = Options()
        # self.options.headless = True
        self.options.add_argument('--no-sandbox')
        # self.options.add_argument("--disable-gpu")
        self.options.add_argument('--disable-dev-shm-usage')
        self.options.add_experimental_option('excludeSwitches', ['enable-logging'])

        # self.chromedriver_path = 'D:/ChromeDriver/chromedriver.exe'
        self.driver = webdriver.Chrome(ChromeDriverManager().install(), options=self.options, service_log_path='NUL')
        self.driver.implicitly_wait(30)
        self.driver.maximize_window()

    def __enter__(self):
        return self.driver

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.driver is not None:
            self.driver.close()
            self.driver.quit()


if __name__ == "__main__":
    os.system("taskkill /f /im chromedriver.exe /T")
