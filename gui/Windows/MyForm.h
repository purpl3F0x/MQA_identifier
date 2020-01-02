#pragma once

#using <System.dll>


namespace mqagui {


using namespace System;
using namespace System::Diagnostics;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

/// <summary>
/// Summary for MyForm
/// </summary>
public ref class MyForm : public System::Windows::Forms::Form {
 public:
  MyForm(void) {
      InitializeComponent();
      //
      //TODO: Add the constructor code here
      //
  }

 protected:
  /// <summary>
  /// Clean up any resources being used.
  /// </summary>
  ~MyForm() {
      if (components) {
          delete components;
      }
  }

 private:
  System::Windows::Forms::Button^fileBtn;
 private:
  System::Windows::Forms::Button^folderBtn;
 protected:

 protected:

 private:
  System::Windows::Forms::TextBox^outputTextBox;


 private:
  /// <summary>
  /// Required designer variable.
  /// </summary>
  System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code

  /// <summary>
  /// Required method for Designer support - do not modify
  /// the contents of this method with the code editor.
  /// </summary>
  void InitializeComponent(void) {
      this->fileBtn = (gcnew
      System::Windows::Forms::Button());
      this->folderBtn = (gcnew
      System::Windows::Forms::Button());
      this->outputTextBox = (gcnew
      System::Windows::Forms::TextBox());
      this->SuspendLayout();
      //
      // fileBtn
      //
      this->fileBtn->Location = System::Drawing::Point(12, 12);
      this->fileBtn->Name = L"fileBtn";
      this->fileBtn->Size = System::Drawing::Size(75, 23);
      this->fileBtn->TabIndex = 0;
      this->fileBtn->Text = L"Open file(s)";
      this->fileBtn->UseVisualStyleBackColor = true;
      this->fileBtn->Click += gcnew
      System::EventHandler(this, &MyForm::fileBtn_Click);
      //
      // folderBtn
      //
      this->folderBtn->Location = System::Drawing::Point(93, 12);
      this->folderBtn->Name = L"folderBtn";
      this->folderBtn->Size = System::Drawing::Size(75, 23);
      this->folderBtn->TabIndex = 1;
      this->folderBtn->Text = L"Open Folder";
      this->folderBtn->UseVisualStyleBackColor = true;
      this->folderBtn->Click += gcnew
      System::EventHandler(this, &MyForm::folderBtn_Click);
      //
      // outputTextBox
      //
      this->outputTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((
          ((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
              | System::Windows::Forms::AnchorStyles::Left)
              | System::Windows::Forms::AnchorStyles::Right));
      this->outputTextBox->BackColor = System::Drawing::SystemColors::Window;
      this->outputTextBox->Font = (gcnew
      System::Drawing::Font(L"Lucida Console",
                            9.75F,
                            System::Drawing::FontStyle::Regular,
                            System::Drawing::GraphicsUnit::Point,
                            static_cast<System::Byte>(0)));
      this->outputTextBox->Location = System::Drawing::Point(13, 42);
      this->outputTextBox->Multiline = true;
      this->outputTextBox->Name = L"outputTextBox";
      this->outputTextBox->ReadOnly = true;
      this->outputTextBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
      this->outputTextBox->Size = System::Drawing::Size(762, 378);
      this->outputTextBox->TabIndex = 2;
      //
      // MyForm
      //
      this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
      this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
      this->ClientSize = System::Drawing::Size(787, 432);
      this->Controls->Add(this->outputTextBox);
      this->Controls->Add(this->folderBtn);
      this->Controls->Add(this->fileBtn);
      this->Name = L"MyForm";
      this->Text = L"MyForm";
      this->ResumeLayout(false);
      this->PerformLayout();

  }

 private:
  System::Void fileBtn_Click(System::Object^sender, System::EventArgs^e) {

      OpenFileDialog ^ sfd = gcnew
      OpenFileDialog();
      sfd->Multiselect = true;
      sfd->Filter = "Flac Audio Files|*.flac";

      if (sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK)
          return;


      String ^ filenames = gcnew
      String("");

      for
      each(auto
      i
      in
      sfd->FileNames)
      filenames += "\"" + i + "\" ";

      // this->outputTextBox->AppendText(filenames + System::Environment::NewLine);
      this->RunMQAid(filenames);
  }

 private:
  System::Void folderBtn_Click(System::Object^sender, System::EventArgs^e) {
      FolderBrowserDialog ^ sfd = gcnew
      FolderBrowserDialog();


      if (sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK)
          return;


      this->RunMQAid("\"" + sfd->SelectedPath + "\"");

  }

 private:
  void RunMQAid(String^args) {
      Process ^ myProcess = gcnew
      Process;
      myProcess->StartInfo->FileName = "./MQA_identifier.exe";
      myProcess->StartInfo->Arguments = args;
      myProcess->StartInfo->CreateNoWindow = true;
      myProcess->StartInfo->UseShellExecute = false;
      myProcess->StartInfo->RedirectStandardOutput = true;
      myProcess->Start();
      myProcess->OutputDataReceived += gcnew
      DataReceivedEventHandler(this, &MyForm::OutputHandler);

      // Asynchronously read the standard output of the spawned process.
      // This raises OutputDataReceived events for each line of output.
      myProcess->BeginOutputReadLine();
      //myProcess->WaitForExit();

      while (!myProcess->HasExited)
          Application::DoEvents();

      myProcess->Close();
  }

 private:
  delegate void TextUpdateDelegate(String^str);

 private:
  void OutputHandler(System::Object^sender, System::Diagnostics::DataReceivedEventArgs^e) {


      //outputTextBox->AppendText(e->Data + System::Environment::NewLine);
      outputTextBox->BeginInvoke(gcnew
      TextUpdateDelegate(this, &MyForm::UpdateOutput), e->Data );
  }


 private:
  void UpdateOutput(System::String^data) {
      this->outputTextBox->AppendText(data + System::Environment::NewLine);
  }


};
}
