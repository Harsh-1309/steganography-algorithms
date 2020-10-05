#!/usr/bin/env python
# coding: utf-8

# In[218]:


# Importing the OpenCV library 
import cv2 
# Reading the image using imread() function 
image = cv2.imread('a1.png') #use(,0) for directly loading images into gray scale


# will show the image in a window 
cv2.imshow('image', image) 
k = cv2.waitKey(0) & 0xFF
# wait for ESC key to exit 
if k == 27:  
    cv2.destroyAllWindows() 
#image = cv2.resize(image, (10,10)) 


# In[219]:


# Extracting the height and width of an image 
h, w = image.shape[:2] 
# Displaying the height and width 
print("Height = {}, Width = {}".format(h, w)) 


# In[220]:


# Extracting RGB values.  
# Here we have randomly chosen a pixel 
# by passing in 100, 100 for height and width, can be custom in image[,]
(B, G, R) = image[700, 800] 
  
# Displaying the pixel values 
print("R = {}, G = {}, B = {}".format(R, G, B)) 
  
# We can also pass the channel to extract  
# the value for a specific channel 
BC = image[100, 100, 0] 
print("BC = {}".format(BC)) 


# In[221]:


# We use cvtColor, to convert to grayscale 
#gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY) 
  
#cv2.imshow('Grayscale', gray_image) 
#cv2.waitKey(0)   
  
# window shown waits for any key pressing event 
#cv2.destroyAllWindows() 


# In[222]:


#(g) = image[700, 800] 
  
# Displaying the pixel values 
#print("g = {}".format(g)) 


# In[223]:


A=[] #creating lists for range input for private stego key
B=[]
def InputFn(): #function for input
    A.clear()
    B.clear()
    i=0
    while(i<5):
        a=int(input("enter lower limit" )) 
        A.append(a)
        print()
        b=int(input("enter upper limit" ))
        B.append(b)
        print()
        i+=1


# In[224]:


InputFn() #taking input


# In[225]:


#data validation
def dataval():
    for i in range(0,5):
        if A[i+1]<=A[i] or B[i+1]<=B[i] or A[i]<0 or A[i]>255 or B[i]<0 or B[i]>255 or (B[i]-A[i])<32 :
            print("invalid input, enter again:")
            InputFn()
            dataval() 
            break
        else:
            print("the range choosen is :") 
            for i in range(0,5):
                print("(",A[i],",",B[i],")")
            break


# In[226]:


dataval()


# In[227]:


BC = image[100, 100, 0] 
print("BC = {}".format(BC)) 


# In[228]:


# Extracting the height and width of an image 
h, w = image.shape[:2] 
# Displaying the height and width 
print("Height = {}, Width = {}".format(h, w)) 


# In[229]:


s1=0;s2=0;s3=0;s4=0;s5=0
for i in range(0,h):
    for j in range(0,w):
        BC=image[i,j,0]
        if A[0]<=BC<=B[0]:
            s1=s1+1
        if A[1]<=BC<=B[1]:
            s2=s2+1
        if A[2]<=BC<=B[2]:
            s3=s3+1
        if A[3]<=BC<=B[3]:
            s4=s4+1
        if A[4]<=BC<=B[4]:
            s5=s5+1


# In[230]:


N=[]
N.append(s1)
N.append(s2)
N.append(s3)
N.append(s4)
N.append(s5)


# In[231]:


for i in range(0,5):
    print("No of pixels in range",i+1,'is',N[i])


# In[232]:


I=[]
for i in range(0,5):
    m=int(input("enter next maximum no of message bits insertion "))
    I.append(m)


# In[233]:


M=N.copy()
li=[] 
  
for i in range(5): 
      li.append([M[i],i]) 
li.sort(reverse=True) 
sort_index = [] 
  
for x in li: 
      sort_index.append(x[1]) 
  
 #print(sort_index)
for i in range(0,5):
    print(I[i],"bits to be inserted in range", sort_index[i]+1)


# In[234]:


# Converting String to binary 
# Using join() + ord() + format() 
  
# initializing string  
message = input("enter message to hide : ")
  
# printing original string  
print("The original message is : " + str(message)) 
  
# using join() + ord() + format() 
# Converting String to binary 
b_message = ''.join(format(ord(i), 'b') for i in message) 
  
# printing result  
print("The message after binary conversion : " + str(b_message)) 


# In[290]:


b_message


# In[333]:


Message=b_message


# In[237]:


l=int(len(b_message))


# In[238]:


#BC=image[23,45,0]


# In[239]:


#BC


# In[240]:


#BC=image.itemset((23,45,0),160)


# In[241]:


#BC


# In[242]:


#print(BC)


# In[243]:


#image.item(23,45,0)


# In[244]:


#n=160


# In[245]:


#b=bin(n).replace("0b", "") 


# In[246]:


#len(b)


# In[403]:


def rangefinder(BC):    #to substitute bits
    if A[0]<=BC<=B[0]:
        return 'range1'
       
    elif A[1]<=BC & BC<=B[1]:
        return 'range2'
        
    elif A[2]<=BC & BC<=B[2]:          
        return 'range3'
        
    elif A[3]<=BC & BC<=B[3]:  
        return 'range4'
        
    elif A[4]<=BC & BC<=B[4]: 
        return 'range5'

    


# In[248]:



#for i in range (0,5):
 #   h=exec("%s=%d" % ('r'+str(sort_index[i]+1),2))
  #  print(h)


# In[272]:


r1=5;r2=4;r3=3;r4=2;r5=1


# In[459]:


def bitverifier(BC_bin_mod,x):
    BC_bin_mod=list(BC_bin_mod)
    if BC_bin_mod[x]=='0':
        BC_bin_mod[x]='1'
    else :
        BC_bin_mod[x]='0'
    #s=int(BC_bin_mod, 10)
    return ''.join(BC_bin_mod)
    


# In[460]:


def bitchanger(i):
    #for i in range(len(b_message)):
    if rangefinder(BC)=='range1':
        BC_bin=bin(BC).replace("0b","")
        BC_bin_mod=BC_bin[0:3]+b_message[i:i+6]  
            # Convert n to base 2
        s = int(BC_bin_mod, 2)
            
        x=2
        while(rangefinder(s)!='range1' and x>=0):
            BC_bin_mod=bitverifier(BC_bin_mod,x)
            s=int(BC_bin_mod,2)
            x=x-1 
                    
                
        image.itemset((n,j,0),s)    
        i=i+5
            
    elif rangefinder(BC)=='range2':
        BC_bin=bin(BC).replace("0b","")
        BC_bin_mod=BC_bin[0:4]+b_message[i:i+5]  
            # Convert n to base 2
        s = int(BC_bin_mod, 2)
            
        x=3
        while(rangefinder(s)!='range2' and x>=0):
            BC_bin_mod=bitverifier(BC_bin_mod,x)
            s=int(BC_bin_mod,2)
            x=x-1 
        image.itemset((n,j,0),s)
        i=i+4
            
    elif rangefinder(BC)=='range3':
        BC_bin=bin(BC).replace("0b","")
        BC_bin_mod=BC_bin[0:5]+b_message[i:i+4]  
            # Convert n to base 2
        s = int(BC_bin_mod, 2)
            
        x=4
        while(rangefinder(s)!='range3' and x>=0):
            BC_bin_mod=bitverifier(BC_bin_mod,x)
            s=int(BC_bin_mod,2)
            x=x-1 
        image.itemset((n,j,0),s)
        i=i+3
            
    elif rangefinder(BC)=='range4':
        BC_bin=bin(BC).replace("0b","")
        BC_bin_mod=BC_bin[0:6]+b_message[i:i+3]  
            # Convert n to base 2
        s = int(BC_bin_mod, 2)
            
        x=5
        while(rangefinder(s)!='range4' and x>=0):
            BC_bin_mod=bitverifier(BC_bin_mod,x)
            s=int(BC_bin_mod,2)
            x=x-1 
        image.itemset((n,j,0),s)
        i=i+2
        
    elif rangefinder(BC)=='range5':
        BC_bin=bin(BC).replace("0b","")
        BC_bin_mod=BC_bin[0:7]+b_message[i:i+2]  
            # Convert n to base 2
        s = int(BC_bin_mod,2)
            
        x=6
        while(rangefinder(s)!='range5' and x>=0):
            BC_bin_mod=bitverifier(BC_bin_mod,x)
            s=int(BC_bin_mod,2)
            x=x-1 
        image.itemset((n,j,0),s)
        i=i+1
    
    return i

 


# In[461]:


i=0
if l>(h*w):
    print("use a larger image")
    
else :
    for n in range(0,h):
        if i>=len(b_message):
            print("end of string")
            break
        for j in range(0,w):
            if i>=len(b_message):
                print("end of string")
                break
            BC=image[n,j,0]
            rangefinder(BC)
            i=bitchanger(i)
            #if n*j>len(b_message):
                
                #break
            print(rangefinder(BC))    
        #if n*j<len(b_message):
            #print("end of message")
            #break


# In[462]:


B


# In[303]:


stego_image=image


# In[348]:


# will show the image in a window 
cv2.imshow('stego_image', image) 
k = cv2.waitKey(0) & 0xFF
# wait for ESC key to exit 
if k == 27:  
    cv2.destroyAllWindows() 


# In[465]:


len_message=int(input("enter length of message: "))


# In[387]:


def rangefinder_D(BC): 
    print(BC)
    print(A)
    print(B)
    if A[0]<=BC<=B[0]:
        return 'range1'
       
    if A[1]<=BC<=B[1]:
        return 'range2'
        
    if A[2]<=BC<=B[2]:          
        return 'range3'
        
    if A[3]<=BC<=B[3]:  
        return 'range4'
        
    if A[4]<=BC<=B[4]: 
        return 'range5'


# In[388]:


message=[]
message.clear()
def bitchanger_D(i):
    #for i in range(len(b_message)):
    if rangefinder_D(BC)=='range1':
        BC_bin_D=bin(BC).replace("0b","")
        BC_bin_M=BC_bin_D[4:9]  
            # Convert n to base 2
            #s = int(BC_bin_M, 10)
        message.append(BC_bin_M)
        i=i+5
            
    if rangefinder_D(BC)=='range2':
        BC_bin_D=bin(BC).replace("0b","")
        BC_bin_M=BC_bin_D[5:9]  
            # Convert n to base 2
           # s = int(BC_bin_M, 10)
        message.append(BC_bin_M)
        i=i+4
            
    if rangefinder_D(BC)=='range3':
        BC_bin_D=bin(BC).replace("0b","")
        BC_bin_M=BC_bin_D[6:9]  
            # Convert n to base 2
           # s = int(BC_bin_M, 10)
        message.append(BC_bin_M)
        i=i+3
            
    if rangefinder_D(BC)=='range4':
        BC_bin_D=bin(BC).replace("0b","")
        BC_bin_M=BC_bin_D[7:9]  
            # Convert n to base 2
            #s = int(BC_bin_M, 10)
        message.append(BC_bin_M)
        i=i+2    
            
    if rangefinder_D(BC)=='range5':
        BC_bin_D=bin(BC).replace("0b","")
        BC_bin_M=BC_bin_D[8:9]  
            # Convert n to base 2
           # s = int(BC_bin_M, 10)
        message.append(BC_bin_M)
        i=i+1  
            
        #if i>=len(b_message):
           # break
    #print("end of string")
    return i


# In[302]:


image.item(2,2,0)


# In[389]:


i=0
for n in range(0,h):
    if i>=len_message:
        print("end of string")
        break
    for j in range(0,w):
        if i>=len_message:
            print("end of string")
            break
        BC=stego_image[n,j,0]
        
        rangefinder_D(BC)
        i=bitchanger_D(i)
    print(rangefinder_D(BC))    
    if n*j<len(b_message):
        print("end of message")
    break


# In[334]:


print(Message)


# In[335]:


def BinaryToDecimal(binary): 
      
    # Using int function to convert to 
    # string    
    string = int(binary, 2) 
      
    return string 
      
# Driver's code 
# initializing binary data 
bin_data =Message
   
# print binary data 
print("The binary value is:", bin_data) 
   
# initializing a empty string for  
# storing the string data 
str_data =' '
   
# slicing the input and converting it  
# in decimal and then converting it in string 
for i in range(0, len(bin_data), 7): 
      
    # slicing the bin_data from index range [0, 6] 
    # and storing it in temp_data 
    temp_data = bin_data[i:i + 7] 
       
    # passing temp_data in BinarytoDecimal() function 
    # to get decimal value of corresponding temp_data 
    decimal_data = BinaryToDecimal(temp_data) 
       
    # Deccoding the decimal value returned by  
    # BinarytoDecimal() function, using chr()  
    # function which return the string corresponding  
    # character for given ASCII value, and store it  
    # in str_data 
    str_data = str_data + chr(decimal_data)  
   
print("The value after string conversion is:", 
       str_data) 


# In[342]:


if Message == b_message:
    print("data extraction successful")


# In[344]:


print("the message sent was:", str_data)


# In[345]:


print(A)


# In[390]:


print(message)


# In[413]:


BC_bin_mod[3]


# In[ ]:





# In[ ]:




