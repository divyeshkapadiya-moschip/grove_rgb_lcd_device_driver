CC = gcc

# Target executable path
TARGET = lcd_test_app

# Build the target executable
all : $(TARGET) 

# Compile source file to create executable.  
$(TARGET):lcd_test_app.c
	@$(CC) -o $@ $<

#Clean files which is generated.	
clean:
	@rm -rf lcd_test_app
