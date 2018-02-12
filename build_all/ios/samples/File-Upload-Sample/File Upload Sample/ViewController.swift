// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

import UIKit
import AzureIoTHubClient

class ViewController: UIViewController {

    private let connectionString = "<insert connection string here>"
    
    // IoT hub handle
    public var iotHubClientHandle: IOTHUB_CLIENT_HANDLE!;
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    let fileUploadComplete: IOTHUB_CLIENT_FILE_UPLOAD_CALLBACK = { result, userContext in
        
        var mySelf: ViewController = Unmanaged<ViewController>.fromOpaque(userContext!).takeUnretainedValue()
        
        DispatchQueue.main.async(execute: { () -> Void in
            mySelf.uploadReport(result: result)
        })
    }
    
    func uploadReport(result: IOTHUB_CLIENT_FILE_UPLOAD_RESULT) {
        
        btnUpload.isEnabled = true
        
        if result == FILE_UPLOAD_OK {
            txtFilename.text = ""
            txtFileContent.text = ""
            showPopup(title: "Success", message: "File upload complete")
        }
        else {
            showPopup(title: "Error", message: "Failed upload failed")
        }

        IoTHubClient_Destroy(iotHubClientHandle)
    }
    
    @IBOutlet weak var btnUpload: UIButton!
    @IBOutlet weak var txtFilename: UITextField!
    @IBOutlet weak var txtFileContent: UITextField!
    
    func showPopup(title: String, message: String) {
        let alertController = UIAlertController(title: title,
                                                message: message,
                                                preferredStyle: UIAlertControllerStyle.alert)
        alertController.addAction(UIAlertAction(title: "OK", style: UIAlertActionStyle.default, handler:nil))
        present(alertController, animated: true, completion: nil)
        
    }
    
    @IBAction func uploadFile(sender: UIButton) {
        print(txtFilename.text ?? "")
        print(txtFileContent.text ?? "")
        
        if ((txtFilename.text?.isEmpty)! || (txtFileContent.text?.isEmpty)!) {
            showPopup(title: "Error", message: "You must provide a file name and content")
            return
        }

        btnUpload.isEnabled = false
        iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString, HTTP_Protocol)
        
        if (iotHubClientHandle == nil) {
            showPopup(title: "Error", message: "Failed to obtain an IoT hub handle")
            return
        }
        
        // Mangle my self pointer in order to pass it as an UnsafeMutableRawPointer
        let that = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())
        let pFileName = strdup((txtFilename.text ?? ""))            // UnsafePointer<Int8>
        let pFilecontent = [UInt8](txtFileContent.text!.utf8)       // UnsafePointer<UInt8>
        
        if (IoTHubClient_UploadToBlobAsync(iotHubClientHandle, pFileName, pFilecontent, (txtFileContent.text?.lengthOfBytes(using: String.Encoding.utf8))!, fileUploadComplete, that) != IOTHUB_CLIENT_OK) {
            showPopup(title: "Error", message: "File upload failed")
            btnUpload.isEnabled = true
        }
    }
}

