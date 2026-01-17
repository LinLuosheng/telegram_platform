package com.yupi.springbootinit.service.impl;

import com.yupi.springbootinit.service.OcrService;
import net.sourceforge.tess4j.Tesseract;
import net.sourceforge.tess4j.TesseractException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.stereotype.Service;

import java.io.File;

@Service
@ConditionalOnProperty(name = "ocr.provider", havingValue = "tesseract", matchIfMissing = true)
public class TesseractOcrServiceImpl implements OcrService {

    private static final Logger log = LoggerFactory.getLogger(TesseractOcrServiceImpl.class);

    @Override
    public String doOcr(File imageFile) {
        if (imageFile == null || !imageFile.exists()) {
            log.warn("OCR skipped: File is null or does not exist");
            return "";
        }
        
        Tesseract tesseract = new Tesseract();
        
        // Explicitly set tessdata path if it exists
        String rootPath = System.getProperty("user.dir");
        File tessDataFolder = new File(rootPath, "tessdata");
        if (tessDataFolder.exists() && tessDataFolder.isDirectory() && tessDataFolder.list() != null && tessDataFolder.list().length > 0) {
            tesseract.setDatapath(tessDataFolder.getAbsolutePath());
            log.info("Using local tessdata path: {}", tessDataFolder.getAbsolutePath());
        } else {
            log.info("Local tessdata not found or empty at {}, using system default.", tessDataFolder.getAbsolutePath());
        }

        // Set language to Chinese + English
        tesseract.setLanguage("chi_sim+eng");
        
        try {
            log.info("Starting Tesseract OCR for file: {}", imageFile.getName());
            String text = tesseract.doOCR(imageFile);
            log.info("OCR Result length: {}", text.length());
            return text;
        } catch (TesseractException e) {
            log.error("OCR Failed", e);
            return "";
        } catch (UnsatisfiedLinkError e) {
            log.error("OCR Native Library missing. Please install Visual C++ Redistributable.", e);
            return "";
        } catch (Throwable e) {
            log.error("OCR Error (Throwable)", e);
            return "";
        }
    }
}
