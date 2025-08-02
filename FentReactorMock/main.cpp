// FentReactorMock.cpp : Defines the entry point for the application.
//

#include "ImageGenerator.h"

int main(int argc, char* argv[]) {
    ImageGenerator app;

    // Check for command line arguments (for Python wrapper)
    if (argc >= 3) {
        std::string prompt = argv[1];
        std::string style = argv[2];

        std::cout << "Running in command-line mode" << std::endl;
        app.runCommandLine(prompt, style);
    }
    else {
        // Run in GUI mode
        std::cout << "Running in GUI mode" << std::endl;
        std::cout << "Usage for command-line: ./image_generator \"<prompt>\" \"<style>\"" << std::endl;
        std::cout << "Styles: photorealistic, artistic, cartoon, abstract, vintage" << std::endl;
        app.run();
    }

    return 0;
}
