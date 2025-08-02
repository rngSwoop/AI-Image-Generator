#define _CRT_SECURE_NO_WARNINGS
#include "ImageGenerator.h"
#include <chrono>
#include <fstream>

using json = nlohmann::json;

ImageGenerator::ImageGenerator() : window(sf::VideoMode({ 1024, 768 }), "AI Image Generator"),
currentState(AppState::INPUT_SCREEN),
selectedStyle(StyleMode::NONE),
selectedModel(APIModel::REALISM),
promptActive(false),
backgroundColor(sf::Color(40, 40, 40)),
buttonColor(sf::Color(70, 70, 70)),
buttonHoverColor(sf::Color(90, 90, 90)),
selectedButtonColor(sf::Color(100, 150, 200)),
disabledButtonColor(sf::Color(50, 50, 50)),
promptLabel(font),
promptText(font),
ghibliLabel(font),
photorealisticLabel(font),
generateLabel(font),
newImageLabel(font),
loadingText(font) {

    if (!font.openFromFile("Yrsa-Regular.ttf")) {
        // Try to load a system font as fallback
        if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            std::cout << "Warning: Could not load Yrsa-Regular.ttf or arial.ttf, using default font" << std::endl;
        }
    }

    modelNames = { "Realism", "Aesthetic" };
    initializeUI();
}

void ImageGenerator::initializeUI() {
    // Prompt label
    promptLabel.setFont(font);
    promptLabel.setString("Enter your image prompt:");
    promptLabel.setCharacterSize(24);
    promptLabel.setFillColor(sf::Color::White);
    promptLabel.setPosition({ 50, 50 });

    // Prompt input box
    promptBox.setSize({ 924, 50 });
    promptBox.setPosition({ 50, 90 });
    promptBox.setFillColor(sf::Color(60, 60, 60));
    promptBox.setOutlineThickness(2);
    promptBox.setOutlineColor(sf::Color(100, 100, 100));

    // Prompt text
    promptText.setFont(font);
    promptText.setCharacterSize(20);
    promptText.setFillColor(sf::Color::White);
    promptText.setPosition({ 60, 105 });

    // Model selection buttons (Realism vs Aesthetic)
    for (int i = 0; i < 2; i++) {
        sf::RectangleShape button({ 200, 50 });
        button.setPosition({ 312 + i * 220.0f, 140 });
        button.setFillColor(i == 0 ? selectedButtonColor : buttonColor);
        modelButtons.push_back(button);

        sf::Text label(font);
        label.setString(modelNames[i]);
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition({ 312 + i * 220.0f + (200 - textBounds.size.x) / 2,
                        140 + (50 - textBounds.size.y) / 2 - 5 });
        modelLabels.push_back(label);
    }

    // Style buttons - Studio Ghibli and Photorealistic
    ghibliButton.setSize({ 200, 60 });
    ghibliButton.setPosition({ 312, 200 });
    ghibliButton.setFillColor(buttonColor);

    ghibliLabel.setFont(font);
    ghibliLabel.setString("Studio Ghibli");
    ghibliLabel.setCharacterSize(18);
    ghibliLabel.setFillColor(sf::Color::White);
    sf::FloatRect ghibliBounds = ghibliLabel.getLocalBounds();
    ghibliLabel.setPosition({ 312 + (200 - ghibliBounds.size.x) / 2,
                    200 + (60 - ghibliBounds.size.y) / 2 - 5 });

    photorealisticButton.setSize({ 200, 60 });
    photorealisticButton.setPosition({ 532, 200 });
    photorealisticButton.setFillColor(buttonColor);

    photorealisticLabel.setFont(font);
    photorealisticLabel.setString("Photorealistic");
    photorealisticLabel.setCharacterSize(18);
    photorealisticLabel.setFillColor(sf::Color::White);
    sf::FloatRect photoBounds = photorealisticLabel.getLocalBounds();
    photorealisticLabel.setPosition({ 532 + (200 - photoBounds.size.x) / 2,
                    200 + (60 - photoBounds.size.y) / 2 - 5 });

    // Generate button
    generateButton.setSize({ 200, 60 });
    generateButton.setPosition({ 412, 280 });
    generateButton.setFillColor(sf::Color(50, 150, 50));

    generateLabel.setFont(font);
    generateLabel.setString("Generate Image");
    generateLabel.setCharacterSize(20);
    generateLabel.setFillColor(sf::Color::White);
    sf::FloatRect genBounds = generateLabel.getLocalBounds();
    generateLabel.setPosition({ 412 + (200 - genBounds.size.x) / 2, 295 });

    // New image button (for image display screen)
    newImageButton.setSize({ 200, 60 });
    newImageButton.setPosition({ 774, 648 });
    newImageButton.setFillColor(sf::Color(50, 150, 50));

    newImageLabel.setFont(font);
    newImageLabel.setString("New Image");
    newImageLabel.setCharacterSize(20);
    newImageLabel.setFillColor(sf::Color::White);
    sf::FloatRect newBounds = newImageLabel.getLocalBounds();
    newImageLabel.setPosition({ 774 + (200 - newBounds.size.x) / 2, 665 });

    // Loading text
    loadingText.setFont(font);
    loadingText.setString("Generating image...");
    loadingText.setCharacterSize(32);
    loadingText.setFillColor(sf::Color::White);
    sf::FloatRect loadBounds = loadingText.getLocalBounds();
    loadingText.setPosition({ (1024 - loadBounds.size.x) / 2,
                           (768 - loadBounds.size.y) / 2 });
}

void ImageGenerator::handleEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (currentState == AppState::INPUT_SCREEN) {
            handleInputScreenEvents(*event);
        }
        else if (currentState == AppState::IMAGE_DISPLAY) {
            handleImageDisplayEvents(*event);
        }
    }
}

void ImageGenerator::handleInputScreenEvents(sf::Event& event) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        // Check prompt box click
        if (promptBox.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            promptActive = true;
            promptBox.setOutlineColor(sf::Color::Blue);
        }
        else {
            promptActive = false;
            promptBox.setOutlineColor(sf::Color(100, 100, 100));
        }

        // Check model button clicks
        for (int i = 0; i < modelButtons.size(); i++) {
            if (modelButtons[i].getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                // Don't allow switching to Aesthetic if Photorealistic is selected
                if (i == 1 && selectedStyle == StyleMode::PHOTOREALISTIC) {
                    continue; // Block the click
                }

                selectedModel = static_cast<APIModel>(i);
                updateModelButtons();
            }
        }

        // Check Studio Ghibli button click
        if (ghibliButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            selectedStyle = (selectedStyle == StyleMode::STUDIO_GHIBLI) ? StyleMode::NONE : StyleMode::STUDIO_GHIBLI;
            updateStyleButtons();
        }

        // Check Photorealistic button click
        if (photorealisticButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            selectedStyle = (selectedStyle == StyleMode::PHOTOREALISTIC) ? StyleMode::NONE : StyleMode::PHOTOREALISTIC;

            // Auto-switch to Realism if Photorealistic is selected
            if (selectedStyle == StyleMode::PHOTOREALISTIC) {
                selectedModel = APIModel::REALISM;
                updateModelButtons();
                std::cout << "Auto-switched to Realism API for photorealistic style" << std::endl;
            }

            updateStyleButtons();
        }

        // Check generate button
        if (generateButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            if (!userPrompt.empty()) {
                generateImage();
            }
        }
    }

    // Handle text input
    if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
        if (promptActive) {
            if (textEntered->unicode == 8) { // Backspace
                if (!userPrompt.empty()) {
                    userPrompt.pop_back();
                    promptText.setString(userPrompt);
                }
            }
            else if (textEntered->unicode >= 32 && textEntered->unicode < 127) {
                if (userPrompt.length() < 100) { // Limit prompt length
                    userPrompt += static_cast<char>(textEntered->unicode);
                    promptText.setString(userPrompt);
                }
            }
        }
    }

    // Update button hover effects
    updateButtonHovers(mousePos);
}

void ImageGenerator::handleImageDisplayEvents(sf::Event& event) {
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        if (newImageButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            currentState = AppState::INPUT_SCREEN;
        }
    }
}

void ImageGenerator::updateStyleButtons() {
    // Update Studio Ghibli button
    if (selectedStyle == StyleMode::STUDIO_GHIBLI) {
        ghibliButton.setFillColor(selectedButtonColor);
    }
    else {
        ghibliButton.setFillColor(buttonColor);
    }

    // Update Photorealistic button
    if (selectedStyle == StyleMode::PHOTOREALISTIC) {
        photorealisticButton.setFillColor(selectedButtonColor);
    }
    else {
        photorealisticButton.setFillColor(buttonColor);
    }
}

void ImageGenerator::updateModelButtons() {
    for (int i = 0; i < modelButtons.size(); i++) {
        if (i == static_cast<int>(selectedModel)) {
            modelButtons[i].setFillColor(selectedButtonColor);
        }
        else if (i == 1 && selectedStyle == StyleMode::PHOTOREALISTIC) {
            // Disable Aesthetic button when Photorealistic is selected
            modelButtons[i].setFillColor(disabledButtonColor);
        }
        else {
            modelButtons[i].setFillColor(buttonColor);
        }
    }
}

void ImageGenerator::updateButtonHovers(sf::Vector2i mousePos) {
    // Generate button hover
    if (generateButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
        generateButton.setFillColor(sf::Color(70, 170, 70));
    }
    else {
        generateButton.setFillColor(sf::Color(50, 150, 50));
    }

    // Studio Ghibli button hover
    if (ghibliButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
        if (selectedStyle != StyleMode::STUDIO_GHIBLI) {
            ghibliButton.setFillColor(buttonHoverColor);
        }
    }
    else {
        if (selectedStyle != StyleMode::STUDIO_GHIBLI) {
            ghibliButton.setFillColor(buttonColor);
        }
    }

    // Photorealistic button hover
    if (photorealisticButton.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
        if (selectedStyle != StyleMode::PHOTOREALISTIC) {
            photorealisticButton.setFillColor(buttonHoverColor);
        }
    }
    else {
        if (selectedStyle != StyleMode::PHOTOREALISTIC) {
            photorealisticButton.setFillColor(buttonColor);
        }
    }

    // Model button hovers
    for (int i = 0; i < modelButtons.size(); i++) {
        if (modelButtons[i].getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            if (i != static_cast<int>(selectedModel) &&
                !(i == 1 && selectedStyle == StyleMode::PHOTOREALISTIC)) {
                modelButtons[i].setFillColor(buttonHoverColor);
            }
        }
        else {
            if (i != static_cast<int>(selectedModel)) {
                if (i == 1 && selectedStyle == StyleMode::PHOTOREALISTIC) {
                    modelButtons[i].setFillColor(disabledButtonColor);
                }
                else {
                    modelButtons[i].setFillColor(buttonColor);
                }
            }
        }
    }
}

void ImageGenerator::generateImage() {
    currentState = AppState::LOADING;

    std::thread([this]() {
        std::cout << "Starting API request..." << std::endl;

        // Make API request using selected model
        std::string requestId = makeAPIRequest(userPrompt, getStylePromptModifier(selectedStyle), selectedModel);

        if (requestId.empty()) {
            std::cout << "Failed to submit API request" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        std::cout << "Request submitted with ID: " << requestId << std::endl;

        // Poll for completion
        std::string imageUrl;
        int maxAttempts = 60; // 60 attempts * 2 seconds = 2 minutes max

        for (int attempt = 0; attempt < maxAttempts; attempt++) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::string statusResponse = pollRequestStatus(requestId, selectedModel);
            if (statusResponse.empty()) {
                std::cout << "Failed to get status" << std::endl;
                continue;
            }

            try {
                json statusJson = json::parse(statusResponse);

                // Check if the response contains images (meaning it's completed)
                if (statusJson.contains("images") && statusJson["images"].is_array() && !statusJson["images"].empty()) {
                    imageUrl = statusJson["images"][0]["url"];
                    std::cout << "Image generation completed!" << std::endl;
                    break;
                }

                std::string status = "UNKNOWN";

                // Handle status field if it exists
                if (statusJson.contains("status") && statusJson["status"].is_string()) {
                    status = statusJson["status"];
                }
                else if (statusJson.contains("status") && statusJson["status"].is_null()) {
                    status = "QUEUED";
                }
                else {
                    // If no status field but no images either, still processing
                    status = "PROCESSING";
                }

                std::cout << "Status: " << status << " (attempt " << (attempt + 1) << "/" << maxAttempts << ")" << std::endl;

                if (status == "FAILED") {
                    std::cout << "API request failed" << std::endl;
                    currentState = AppState::INPUT_SCREEN;
                    return;
                }
                // Continue polling for any other status
            }
            catch (const std::exception& e) {
                std::cout << "Error parsing status: " << e.what() << std::endl;
                std::cout << "Raw response: '" << statusResponse << "'" << std::endl;

                // If we can't parse the response, it might be an HTTP error
                if (statusResponse.find("404") != std::string::npos ||
                    statusResponse.find("Not Found") != std::string::npos) {
                    std::cout << "Endpoint not found - check API URL" << std::endl;
                    currentState = AppState::INPUT_SCREEN;
                    return;
                }
            }
        }

        if (imageUrl.empty()) {
            std::cout << "Timeout waiting for image generation" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        std::cout << "Image URL received: " << imageUrl << std::endl;

        // Download image
        std::string filename = "generated_image.jpg";
        if (!downloadImage(imageUrl, filename)) {
            std::cout << "Failed to download image" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        // Load image into SFML
        if (imageTexture.loadFromFile(filename)) {
            std::cout << "High-resolution image loaded successfully" << std::endl;
            std::cout << "Image resolution: " << imageTexture.getSize().x << "x" << imageTexture.getSize().y << std::endl;
            imageSprite.setTexture(imageTexture, true);

            // Scale image to fit screen while maintaining aspect ratio
            float scaleX = 1024.0f / imageTexture.getSize().x;
            float scaleY = 768.0f / imageTexture.getSize().y;
            float scale = std::min(scaleX, scaleY);
            imageSprite.setScale({ scale, scale });

            // Center the image
            sf::FloatRect spriteBounds = imageSprite.getGlobalBounds();
            float posX = (1024 - spriteBounds.size.x) / 2;
            float posY = (768 - spriteBounds.size.y) / 2;
            imageSprite.setPosition({ posX, posY });

            std::cout << "Image positioned and scaled" << std::endl;
        }
        else {
            std::cout << "Failed to load image file" << std::endl;
            currentState = AppState::INPUT_SCREEN;
            return;
        }

        currentState = AppState::IMAGE_DISPLAY;
        }).detach();
}

void ImageGenerator::render() {
    window.clear(backgroundColor);

    switch (currentState) {
    case AppState::INPUT_SCREEN:
        renderInputScreen();
        break;
    case AppState::LOADING:
        renderLoadingScreen();
        break;
    case AppState::IMAGE_DISPLAY:
        renderImageDisplay();
        break;
    }

    window.display();
}

void ImageGenerator::renderInputScreen() {
    window.draw(promptLabel);
    window.draw(promptBox);
    window.draw(promptText);

    for (const auto& button : modelButtons) {
        window.draw(button);
    }
    for (const auto& label : modelLabels) {
        window.draw(label);
    }

    window.draw(ghibliButton);
    window.draw(ghibliLabel);
    window.draw(photorealisticButton);
    window.draw(photorealisticLabel);

    window.draw(generateButton);
    window.draw(generateLabel);
}

void ImageGenerator::renderLoadingScreen() {
    window.draw(loadingText);
}

void ImageGenerator::renderImageDisplay() {
    window.draw(imageSprite);
    window.draw(newImageButton);
    window.draw(newImageLabel);
}

// HTTP callback function
size_t ImageGenerator::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}

std::string ImageGenerator::makeAPIRequest(const std::string& prompt, const std::string& styleModifier, APIModel model) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    // Get API key from environment variable
    const char* apiKey = std::getenv("FAL_KEY");
    if (!apiKey) {
        std::cout << "ERROR: FAL_KEY environment variable not set!" << std::endl;
        curl_easy_cleanup(curl);
        return "";
    }

    // Prepare headers
    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Key " + std::string(apiKey);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, authHeader.c_str());

    // Prepare JSON payload based on selected model - HIGH RESOLUTION
    json payload;
    std::string apiUrl;

    if (model == APIModel::REALISM) {
        // FLUX schnell API - Custom high resolution
        apiUrl = "https://queue.fal.run/fal-ai/flux-1/schnell";
        payload = {
            {"prompt", prompt + styleModifier},
            {"image_size", {{"width", 1296}, {"height", 2304}}}, // High res 9:16 ratio
            {"num_inference_steps", 4},
            {"guidance_scale", 3.5},
            {"num_images", 1},
            {"enable_safety_checker", true},
            {"output_format", "jpeg"}
        };
    }
    else {
        // Playground v2.5 API - High resolution 9:16 aspect ratio  
        apiUrl = "https://queue.fal.run/fal-ai/playground-v25";
        payload = {
            {"prompt", prompt + styleModifier},
            {"image_size", {{"width", 1296}, {"height", 2304}}}, // High res 9:16 ratio
            {"num_images", 1},
            {"guidance_scale", 7},
            {"format", "jpeg"},
            {"num_inference_steps", 50}
        };
    }

    std::string jsonString = payload.dump();

    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Make request
    res = curl_easy_perform(curl);

    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    try {
        json responseJson = json::parse(response);
        if (responseJson.contains("request_id")) {
            return responseJson["request_id"];
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error parsing API response: " << e.what() << std::endl;
        std::cout << "Response: " << response << std::endl;
    }

    return "";
}

std::string ImageGenerator::pollRequestStatus(const std::string& requestId, APIModel model) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    const char* apiKey = std::getenv("FAL_KEY");
    if (!apiKey) {
        curl_easy_cleanup(curl);
        return "";
    }

    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Key " + std::string(apiKey);
    headers = curl_slist_append(headers, authHeader.c_str());

    // Build URL based on model
    std::string url;
    if (model == APIModel::REALISM) {
        url = "https://queue.fal.run/fal-ai/flux-1/requests/" + requestId;
    }
    else {
        url = "https://queue.fal.run/fal-ai/playground-v25/requests/" + requestId;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }

    return response;
}

bool ImageGenerator::downloadImage(const std::string& imageUrl, const std::string& filename) {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, imageUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}

std::string ImageGenerator::getStylePromptModifier(StyleMode style) {
    switch (style) {
    case StyleMode::STUDIO_GHIBLI:
        return ", in the style of Studio Ghibli";
    case StyleMode::PHOTOREALISTIC:
        return ", photorealistic photography, ultra realistic, highly detailed, 8k resolution, professional photography, sharp focus, real world";
    case StyleMode::NONE:
    default:
        return "";
    }
}

void ImageGenerator::run() {
    while (window.isOpen()) {
        handleEvents();
        render();
    }
}

void ImageGenerator::runCommandLine(const std::string& prompt, const std::string& style) {
    std::cout << "Processing prompt: " << prompt << std::endl;
    std::cout << "Style: " << style << std::endl;

    // Set the prompt and style
    userPrompt = prompt;

    // Convert style string to enum
    if (style == "studio_ghibli") selectedStyle = StyleMode::STUDIO_GHIBLI;
    else if (style == "photorealistic") selectedStyle = StyleMode::PHOTOREALISTIC;
    else selectedStyle = StyleMode::NONE;

    // Generate and display image
    generateImage();

    // Wait for generation to complete
    while (currentState == AppState::LOADING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    currentState = AppState::IMAGE_DISPLAY;
    run(); // Show GUI with generated image
}