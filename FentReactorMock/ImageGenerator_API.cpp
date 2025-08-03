#include "ImageGenerator.h"
#include <chrono>
#include <fstream>

using json = nlohmann::json;

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