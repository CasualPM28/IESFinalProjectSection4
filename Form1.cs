using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CSharpFinalProjectIES
{
    public partial class Form1 : Form
    {
        int startflag = 0;
        int flag_sensor;
        string RxString;
        string temp = "30";
        string light = "30";
        string humidity = "30";
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object Sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
                serialPort1.Close();

            serialPort1.PortName = "COM4";
            serialPort1.BaudRate = 9600;
        }

        private void SerialStart_Click(object sender, EventArgs e)
        {
            serialPort1.PortName = "COM4";
            serialPort1.BaudRate = 9600;

            serialPort1.Open();
            if(serialPort1.IsOpen)
            {
                textBox1.ReadOnly = false;
                textBox2.ReadOnly = false;
                textBox3.ReadOnly = false;
            }
        }

        private void SerialStop_Click(object sender, EventArgs e)
        {
            serialPort1.Close();
            textBox1.ReadOnly = true;
            textBox2.ReadOnly= true;
            textBox3.ReadOnly= true;
        }

        private void ReadInThingSpeak_Click(object sender, EventArgs e)
        {
            WebClient client = new WebClient();

            CurrentData.Text = client.DownloadString("http://api.thingspeak.com/channels/1363742/field/field1/last.text");
            label3.Text = client.DownloadString("http://api.thingspeak.com/channels/1363742/field/field2/last.text");
            label4.Text = client.DownloadString("http://api.thingspeak.com/channels/1363742/field/field3/last.text");

        }

        private void CurrentData_Click(object sender, EventArgs e)
        {
            textBox1.AppendText(RxString);
            textBox2.AppendText(RxString);
            textBox3.AppendText(RxString);
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (!string.Equals(textBox1.Text, "") || !string.Equals(textBox2.Text, "") || !string.Equals(textBox3.Text, ""));
                if (serialPort1.IsOpen) serialPort1.Close();
                try
                {
                    const string WRITEKEY = "O66EMWU2XZ8P7WGC";
                    string strUpdateBase = "http://api.thingspeak.com/update";
                    string strUpdateURI = strUpdateBase + "?api_key=" + WRITEKEY;
                    string strField1 = textBox1.Text;
                    string strField2 = textBox2.Text;
                    string strField3 = textBox3.Text;
                    HttpWebRequest ThingsSpeakReq;
                    HttpWebResponse ThingsSpeakResp;
                    strUpdateURI += "&field1=" + strField1;
                    strUpdateURI += "&field2=" + strField2;
                    strUpdateURI += "&field3=" + strField3;

                strUpdateURI += "&field1=" + strField1 + "&field2=" + strField2 + "&field3=" + strField3; 
                    flag_sensor++;
                    ThingsSpeakReq = (HttpWebRequest)WebRequest.Create(strUpdateURI);
                    ThingsSpeakResp = (HttpWebResponse)ThingsSpeakReq.GetResponse();
                    ThingsSpeakResp.Close();
                    if (!(string.Equals(ThingsSpeakResp.StatusDescription, "OK")))
                    {
                        Exception exData = new Exception(ThingsSpeakResp.StatusDescription);
                        throw exData;
                    }
                }
                catch (Exception ex)
                {
                }
                textBox1.Text = "";
                textBox2.Text = "";
                textBox3.Text = "";
                serialPort1.Open();
            }
        private void SerialPort1_DataReceived1(object Sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            RxString = serialPort1.ReadExisting();
            this.Invoke(new EventHandler(CurrentData_Click));

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void textBox3_TextChanged(object sender, EventArgs e)
        {
            RxString = serialPort1.ReadExisting();
            this.Invoke(new EventHandler(CurrentData_Click));
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {
            RxString = serialPort1.ReadExisting();
            this.Invoke(new EventHandler(CurrentData_Click));
        }

        private void label3_Click(object sender, EventArgs e)
        {
            textBox2.AppendText(RxString);
        }

        private void label4_Click(object sender, EventArgs e)
        {
            textBox3.AppendText(RxString);
        }

        private void label2_Click(object sender, EventArgs e)
        {

        }
    }
}
