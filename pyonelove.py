import time

import cv2

import imutils

import numpy as np

import pytesseract

from pytesseract import Output

from PIL import Image

import re

img = cv2.imread('4.jpg', cv2.IMREAD_COLOR)

# img = cv2.resize(img, (620,480) )

# ~ sharpen_filter = np.array([[-1, -1, -1],
# ~ [-1, 9, -1],
# ~ [-1, -1, -1]])

# ~ sharpen_img = cv2.filter2D(img, -1, sharpen_filter)

gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)  # convert to grey scale

gray = cv2.bilateralFilter(gray, 11, 17, 17)  # Blur to reduce noise

edged = cv2.Canny(gray, 30, 200)  # Perform Edge detection

# find contours in the edged image, keep only the largest

# ones, and initialize our screen contour

cnts = cv2.findContours(edged.copy(), cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

cnts = imutils.grab_contours(cnts)

cnts = sorted(cnts, key=cv2.contourArea, reverse=True)[:10]

screenCnt = None

# loop over our contours

for c in cnts:

    # approximate the contour

    peri = cv2.arcLength(c, True)

    approx = cv2.approxPolyDP(c, 0.018 * peri, True)

    # if our approximated contour has four points, then

    # we can assume that we have found our screen

    if len(approx) == 4:
        screenCnt = approx

        break

if screenCnt is None:

    detected = 0

    print("No contour detected")

else:

    detected = 1

if detected == 1:
    cv2.drawContours(img, [screenCnt], -1, (0, 255, 0), 3)

# Masking the part other than the number plate

mask = np.zeros(gray.shape, np.uint8)

new_image = cv2.drawContours(mask, [screenCnt], 0, 255, -1, )

new_image = cv2.bitwise_and(img, img, mask=mask)

# Now crop

(x, y) = np.where(mask == 255)

(topx, topy) = (np.min(x), np.min(y))

(bottomx, bottomy) = (np.max(x), np.max(y))

Cropped = gray[topx:bottomx + 1, topy:bottomy + 1]


# ~ # Calculate skew angle of an image
# ~ def getSkewAngle(cvImage) -> float:
# ~ # Prep image, copy, convert to gray scale, blur, and threshold
# ~ newImage = cvImage.copy()
# ~ gray = cv2.cvtColor(newImage, cv2.COLOR_BGR2GRAY)
# ~ blur = cv2.GaussianBlur(gray, (9, 9), 0)
# ~ thresh = cv2.threshold(blur, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)[1]

# ~ # Apply dilate to merge text into meaningful lines/paragraphs.
# ~ # Use larger kernel on X axis to merge characters into single line, cancelling out any spaces.
# ~ # But use smaller kernel on Y axis to separate between different blocks of text
# ~ kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (30, 5))
# ~ dilate = cv2.dilate(thresh, kernel, iterations=5)

# ~ # Find all contours
# ~ contours, hierarchy = cv2.findContours(dilate, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
# ~ contours = sorted(contours, key = cv2.contourArea, reverse = True)

# ~ # Find largest contour and surround in min area box
# ~ largestContour = contours[0]
# ~ minAreaRect = cv2.minAreaRect(largestContour)

# ~ # Determine the angle. Convert it to the value that was originally used to obtain skewed image
# ~ angle = minAreaRect[-1]
# ~ if angle < -45:
# ~ angle = 90 + angle
# ~ return -1.0 * angle


# ~ def rotateImage(cvImage, angle: float):
# ~ newImage = cvImage.copy()
# ~ (h, w) = newImage.shape[:2]
# ~ center = (w // 2, h // 2)
# ~ M = cv2.getRotationMatrix2D(center, angle, 1.0)
# ~ newImage = cv2.warpAffine(newImage, M, (w, h), flags=cv2.INTER_CUBIC, borderMode=cv2.BORDER_REPLICATE)
# ~ return newImage

# ~ def deskew(cvImage):
# ~ angle = getSkewAngle(cvImage)
# ~ return rotateImage(cvImage, -1.0 * angle)

# Read the number plate

def improve_quality(img):
    thr = cv2.adaptiveThreshold(img, 255, cv2.ADAPTIVE_THRESH_MEAN_C,
                                cv2.THRESH_BINARY, 57, 20)
    thr = cv2.erode(thr, (2, 2), iterations=2)

    return thr


data = pytesseract.image_to_data(Cropped, config='--psm 6', output_type=Output.DICT)
print(data['text'], data['conf'])


def get_text_from_img(img):
    return pytesseract.image_to_string(img, config='--psm 7').strip()


def check_match(text):
    return re.match(r'^[A-Z]{1,3}\s{1}[\d]{2,3}\s{1}[A-Z]{3}$', text)

def replace_err(text):
    pattern = r'^[A-Z]{1,2}\d{2,3}[A-Z]{3}$'
    text = text.replace("-", "").replace("_", "").replace("'", "").replace("|", "").replace(" ", "")

    return text

def checkJud (string):

    jud_list = [
        "B",
        "AB",
        "AR",
        "AG",
        "BC",
        "BH",
        "BN",
        "BT",
        "BV",
        "BR",
        "BZ",
        "CS",
        "CL",
        "CJ",
        "HR",
        "HD",
        "IL",
        "IS",
        "IF",
        "TM",
    ]

    found = 0
    print(string[0] + "   " + string[0][:2])
    for x in jud_list:
        print(x)
        if string[:2] == x:
                found = 1

    return found

text = get_text_from_img(Cropped)

text = replace_err(text)

if check_match(text) is not None and checkJud(text) == 1:
    print("number is correct,and it is" + text)
else:
    print("Wrong number:" + text)

# print(type(re.match(r'^[A-Z]{1,3}\s{1}[\d]{2,3}\s{1}[A-Z]{3}$', text)))
#print("Detected Number is : '" + text + "'")

# if check_match(text) is None:
#	improved_img = improve_quality(Cropped)
#	new_text = check_match(get_text_from_img(improved_img))
#	print(new_text)

#	cv2.imshow("Improved", improved_img)


cv2.imshow('image', img)

# ~ cv2.imshow('sharpened', sharpen_img)

cv2.imshow('Cropped', Cropped)

#cv2.waitKey(0)
time.sleep(5)

cv2.destroyAllWindows()

# ~ To do:
# ~ 1. Filtrarea intrarilor (eliminam caracterele speciale + lowercase, verificam judet, ...)
# ~ 2. Implementare repozare
# ~ 3. Testare (testare pe un dataset)
