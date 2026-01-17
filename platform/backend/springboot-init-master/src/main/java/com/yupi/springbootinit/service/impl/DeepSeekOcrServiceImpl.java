package com.yupi.springbootinit.service.impl;

import cn.hutool.core.codec.Base64;
import cn.hutool.core.io.FileUtil;
import cn.hutool.http.HttpRequest;
import cn.hutool.http.HttpResponse;
import cn.hutool.json.JSONArray;
import cn.hutool.json.JSONObject;
import cn.hutool.json.JSONUtil;
import com.yupi.springbootinit.service.OcrService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.stereotype.Service;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Implementation of OCR Service using DeepSeek (or OpenAI-compatible) API.
 * Currently supports models with Vision capabilities (like GPT-4o, Claude 3.5 Sonnet via wrapper, or local LLaVA/Janus).
 * Note: Standard DeepSeek-V3 API is text-only. This client assumes a Vision-compatible endpoint.
 */
@Service
@ConditionalOnProperty(name = "ocr.provider", havingValue = "deepseek")
public class DeepSeekOcrServiceImpl implements OcrService {

    private static final Logger log = LoggerFactory.getLogger(DeepSeekOcrServiceImpl.class);

    @Value("${ocr.deepseek.base-url:https://api.deepseek.com/v1}")
    private String baseUrl;

    @Value("${ocr.deepseek.api-key:}")
    private String apiKey;

    @Value("${ocr.deepseek.model:deepseek-chat}")
    private String model;

    @Override
    public String doOcr(File imageFile) {
        if (imageFile == null || !imageFile.exists()) {
            log.warn("OCR skipped: File is null or does not exist");
            return "";
        }

        if (apiKey == null || apiKey.isEmpty() || "your_api_key_here".equals(apiKey)) {
            log.error("DeepSeek API Key is not configured. Please set ocr.deepseek.api-key in application.yml");
            return "Error: API Key missing";
        }

        log.info("Starting DeepSeek API OCR for file: {} using model: {}", imageFile.getName(), model);

        try {
            // 1. Encode image to Base64
            String base64Image = Base64.encode(FileUtil.readBytes(imageFile));
            String imageUrl = "data:image/jpeg;base64," + base64Image;

            // 2. Construct Request Body (OpenAI Vision format)
            // {
            //   "model": "...",
            //   "messages": [
            //     {
            //       "role": "user",
            //       "content": [
            //         {"type": "text", "text": "Please transcribe all text in this image."},
            //         {"type": "image_url", "image_url": {"url": "..."}}
            //       ]
            //     }
            //   ]
            // }

            JSONObject requestBody = new JSONObject();
            requestBody.set("model", model);

            JSONArray messages = new JSONArray();
            JSONObject userMessage = new JSONObject();
            userMessage.set("role", "user");

            JSONArray content = new JSONArray();
            
            JSONObject textContent = new JSONObject();
            textContent.set("type", "text");
            textContent.set("text", "Please OCR this image and return ONLY the text found. Do not add markdown formatting or commentary.");
            content.add(textContent);

            JSONObject imageContent = new JSONObject();
            imageContent.set("type", "image_url");
            JSONObject imageUrlObj = new JSONObject();
            imageUrlObj.set("url", imageUrl);
            imageContent.set("image_url", imageUrlObj);
            content.add(imageContent);

            userMessage.set("content", content);
            messages.add(userMessage);

            requestBody.set("messages", messages);

            // 3. Send Request
            String chatEndpoint = baseUrl.endsWith("/") ? baseUrl + "chat/completions" : baseUrl + "/chat/completions";
            // If base-url already includes chat/completions, use it directly (common user mistake handling)
            if (baseUrl.contains("chat/completions")) {
                chatEndpoint = baseUrl;
            }

            log.info("Sending request to: {}", chatEndpoint);

            HttpResponse response = HttpRequest.post(chatEndpoint)
                    .header("Authorization", "Bearer " + apiKey)
                    .header("Content-Type", "application/json")
                    .body(requestBody.toString())
                    .timeout(60000) // 60s timeout for image processing
                    .execute();

            String responseBody = response.body();
            
            if (!response.isOk()) {
                log.error("DeepSeek API Error: Code={}, Body={}", response.getStatus(), responseBody);
                return "Error: API call failed with " + response.getStatus();
            }

            // 4. Parse Response
            JSONObject responseJson = JSONUtil.parseObj(responseBody);
            JSONArray choices = responseJson.getJSONArray("choices");
            if (choices != null && !choices.isEmpty()) {
                JSONObject choice = choices.getJSONObject(0);
                JSONObject message = choice.getJSONObject("message");
                String contentStr = message.getStr("content");
                log.info("OCR Success. Result length: {}", contentStr.length());
                return contentStr;
            } else {
                log.warn("DeepSeek API returned no choices: {}", responseBody);
                return "";
            }

        } catch (Exception e) {
            log.error("DeepSeek OCR Exception", e);
            return "Error: " + e.getMessage();
        }
    }
}
