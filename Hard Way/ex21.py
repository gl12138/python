def math(a, b):
	script = input("What do you want to do?")

	if script == 'divide':
		return divide(x, y)
	else:
		return false

def add(a, b):
	print(f"ADDING {a} + {b}")
	return a + b

def subtract(a, b):
	print(f"SUBTRACTING {a} - {b}")
	return a - b 

def multiply(a, b):
	print(f"MULTIPLYING {a} * {b}")
	return a * b

def divide(a, b):
	print(f"DIVIDING {a} / {b}")
	return a / b

print("Lets do some easy things.\n")




print("What do you want to add?")
x, y = map(int, input().split())
age = add(x, y)
print(f"It is {age}")

print("What do you want to subtract?")
x, y = map(int, input().split())
height = subtract(x, y)
print(f"It's {height}")

print("What do you want to multiply?")
x, y = map(int, input().split())
weight = multiply(x, y)
print(f"It's {weight}")

print("Waht do you want to divide?")
x, y = map(float, input().split())
iq = math(x, y)
print(f"It is {iq}")

print(f"Age: {age}, Height: {height}, Weight: {weight}, IQ: {iq}")

x = map(int, input("A new avaliable:"))
what = add(age, subtract(height, multiply(weight, divide(iq, x))))
print("That is", what)