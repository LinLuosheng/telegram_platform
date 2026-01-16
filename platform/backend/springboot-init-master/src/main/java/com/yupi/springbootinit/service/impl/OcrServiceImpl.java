package com.yupi.springbootinit.service.impl;

import com.yupi.springbootinit.service.OcrService;
import lombok.extern.slf4j.Slf4j;
import net.sourceforge.tess4j.Tesseract;
import net.sourceforge.tess4j.TesseractException;
import org.springframework.stereotype.Service;
import java.io.File;

@Service
// @Slf4j
public class OcrServiceImpl implements OcrService {

    private static final org.slf4j.Logger log = org.slf4j.LoggerFactory.getLogger(OcrServiceImpl.class);

    @Override
    public String doOcr(File imageFile) {
        if (imageFile == null || !imageFile.exists()) {
            log.warn("OCR skipped: File is null or does not exist");
            return "";
        }
        
        Tesseract tesseract = new Tesseract();
        
        // Explicitly set tessdata path
        String rootPath = System.getProperty("user.dir");
        File tessDataFolder = new File(rootPath, "tessdata");
        if (tessDataFolder.exists()) {
            tesseract.setDatapath(tessDataFolder.getAbsolutePath());
            log.info("Using tessdata path: {}", tessDataFolder.getAbsolutePath());
        } else {
            log.warn("tessdata folder not found at: {}", tessDataFolder.getAbsolutePath());
            // Fallback to default behavior or return empty if critical
        }

        // Set language to Chinese + English
        tesseract.setLanguage("chi_sim+eng");
        
        try {
            log.info("Starting OCR for file: {}", imageFile.getName());
            String text = tesseract.doOCR(imageFile);
            log.info("OCR Result length: {}", text.length());
            return text;
        } catch (TesseractException e) {
            log.error("OCR Failed", e);
            return "";
        } catch (UnsatisfiedLinkError e) {
            log.error("OCR Native Library missing. Please install Visual C++ Redistributable.", e);
            return "";
        } catch (Exception e) {
            log.error("OCR Error", e);
            return "";
        }
    }
}
