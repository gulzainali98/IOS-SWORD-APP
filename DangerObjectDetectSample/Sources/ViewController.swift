//
//  ViewController.swift
//  SquareDetect
//
//  Created by Dmytro Hrebeniuk on 11/14/18.
//  Copyright © 2018 SquareDetect. All rights reserved.
//

import UIKit
import DangerObjectDetect

class ViewController: UIViewController {

    private let dangerObjectsDetector = DangerObjectsDetector()
    
    override func viewDidLoad() {
        super.viewDidLoad()

        guard let directoryURL = Bundle.main.url(forResource: "TestImages", withExtension: nil) else {
            return
        }
        
        let files = try? FileManager.default.contentsOfDirectory(atPath: directoryURL.path)
        guard let grayFiles = (files?.filter { $0.contains("gray")} )?.sorted() else {
            return
        }

        for (index, grayFile) in grayFiles.enumerated() {
            let rgbFile = grayFile.replacingOccurrences(of: "gray", with: "rgb")
            
            let grayImageURL = directoryURL.appendingPathComponent(grayFile)
            let rgbImageURL = directoryURL.appendingPathComponent(rgbFile)
            
            if let grayImage = UIImage(contentsOfFile: grayImageURL.path), let rgbImage = UIImage(contentsOfFile: rgbImageURL.path) {
                let result = dangerObjectsDetector.detectObjects(inRGBImage: rgbImage, inGrayImage: grayImage, offsetSize: CGSize(width: 0, height: 0))
                if let url = FileManager.default.urls(for: .cachesDirectory, in: .userDomainMask).first {
                    let imageURL = url.appendingPathComponent("resultImage\(index+1)").appendingPathExtension("png")
                    try? result?.pngData()?.write(to: imageURL)
                    print("result image:\(imageURL)")
                }
            }
        }
    }
}
