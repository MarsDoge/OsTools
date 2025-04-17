import imaplib
import smtplib
import email
import os
import re
import requests
import time
from bs4 import BeautifulSoup
from email.mime.multipart import MIMEMultipart
from email.mime.application import MIMEApplication
from email.mime.text import MIMEText

# =================== 配置区 ===================
EMAIL_USER = '********'
EMAIL_PASS = '***************'
IMAP_SERVER = 'imap.****'
SMTP_SERVER = 'smtp.****'
SMTP_PORT = 465

BASE_FW_URL = 'http://*****/Release/10_BIOS/UDK2018-Release/LoongArch/'
TMP_DOWNLOAD_DIR = '/tmp'

POLL_INTERVAL = 600  # 每隔多少秒检查一次

TEXT_TEMPLATE = """\n您好，\n\n您请求的固件型号：{model}\n\n我们已为您找到最新版本，固件文件已作为附件发送给您。\n\n文件名称：{filename}\n\n如有任何问题，欢迎随时联系支持团队。\n\n祝好，\n自动固件服务\n"""

LIST_TEMPLATE = u"""\n您好，\n\n以下是当前支持的固件型号目录：\n\n{fw_list}\n\n[提示] 您可以通过以下方式获取固件：\n- 回复主题为：FW Request: <型号> （例如 FW Request: TD622E0）\n- 若需再次查看此列表，可发送：FW Request: list\n\n自动固件服务\n"""

# =============== 功能实现 ===============

def get_supported_fw_list():
    try:
        print("正在访问固件列表页面: " + BASE_FW_URL)
        r = requests.get(BASE_FW_URL, timeout=10)
        print("返回状态码: ", r.status_code)

        # 匹配形如 <a href="./TD622E0/"> 的链接并提取目录名
        hrefs = re.findall(r'<a href="\./([A-Za-z0-9\-_]+)/">', r.text)
        print(f"匹配到的固件子目录链接: {hrefs}")

        return hrefs
    except Exception as e:
        print(f"[ERROR] 获取型号目录失败: {e}")
        return []

def fetch_latest_fw_file_url(model):
    model_url = f"{BASE_FW_URL}{model}/"
    try:
        print(f"正在访问固件目录: {model_url}")
        r = requests.get(model_url, timeout=10)
        soup = BeautifulSoup(r.text, 'html.parser')
        links = [a['href'] for a in soup.find_all('a') if a['href'].endswith(('.zip', '.fd'))]
        print(f"发现的固件文件: {links}")
        links.sort(reverse=True)
        if links:
            print(f"选取最新固件文件: {links[0]}")
            return model_url + links[0]
        else:
            print(f"未找到 {model} 的固件文件。")
            return None
    except Exception as e:
        print(f"[ERROR] 拉取最新固件失败: {e}")
        return None

def download_fw_file(url, dest_dir=TMP_DOWNLOAD_DIR):
    local_filename = os.path.join(dest_dir, url.split('/')[-1])
    try:
        with requests.get(url, stream=True, timeout=20) as r:
            r.raise_for_status()
            with open(local_filename, 'wb') as f:
                for chunk in r.iter_content(chunk_size=8192):
                    f.write(chunk)
        print(f"成功下载到: {local_filename}")
        return local_filename
    except Exception as e:
        print(f"[ERROR] 下载失败: {e}")
        return None

def send_email_plain(to_addr, subject, body):
    print(f"发送文本邮件至: {to_addr} / 主题: {subject}")
    body_encoded = body.encode('utf-8', errors='replace').decode('utf-8')
    msg = MIMEText(body_encoded, 'plain', 'utf-8')
    msg['From'] = EMAIL_USER
    msg['To'] = to_addr
    msg['Subject'] = subject
    try:
        if SMTP_PORT == 465:
            smtp = smtplib.SMTP_SSL(SMTP_SERVER, SMTP_PORT)
        else:
            smtp = smtplib.SMTP(SMTP_SERVER, SMTP_PORT)
            smtp.starttls()
        smtp.login(EMAIL_USER, EMAIL_PASS)
        smtp.sendmail(EMAIL_USER, [to_addr], msg.as_string())
        smtp.quit()
        print("文本邮件发送成功")
    except Exception as e:
        print(f"[ERROR] 文本邮件发送失败: {e}")

def send_email_with_attachment(to_addr, subject, body, attachment_path):
    print(f"发送带附件邮件至: {to_addr} / 附件: {attachment_path}")
    body_encoded = body.encode('utf-8', errors='replace').decode('utf-8')
    msg = MIMEMultipart()
    msg['From'] = EMAIL_USER
    msg['To'] = to_addr
    msg['Subject'] = subject
    msg.attach(MIMEText(body_encoded, 'plain', 'utf-8'))
    with open(attachment_path, 'rb') as f:
        part = MIMEApplication(f.read(), Name=os.path.basename(attachment_path))
        part['Content-Disposition'] = f'attachment; filename="{os.path.basename(attachment_path)}"'
        msg.attach(part)
    try:
        if SMTP_PORT == 465:
            smtp = smtplib.SMTP_SSL(SMTP_SERVER, SMTP_PORT)
        else:
            smtp = smtplib.SMTP(SMTP_SERVER, SMTP_PORT)
            smtp.starttls()
        smtp.login(EMAIL_USER, EMAIL_PASS)
        smtp.sendmail(EMAIL_USER, [to_addr], msg.as_string())
        smtp.quit()
        print("带附件邮件发送成功")
    except Exception as e:
        print(f"[ERROR] 附件邮件发送失败: {e}")

def handle_fw_request(subject, sender):
    print(f"处理邮件: {sender} - {subject}")
    fw_match = re.search(r'FW Request:\s*(\S+)', subject)
    supported_fw = get_supported_fw_list()

    if fw_match:
        fw_model = fw_match.group(1).upper()

        if fw_model == 'LIST':
            print("收到固件型号列表请求。")
            body = LIST_TEMPLATE.format(fw_list="\n".join(supported_fw))
            send_email_plain(sender, "Supported Firmware List", body)
            return

        print(f"请求型号为: {fw_model}")
        if fw_model in supported_fw:
            fw_link = fetch_latest_fw_file_url(fw_model)
            if fw_link:
                fw_path = download_fw_file(fw_link)
                if fw_path:
                    body = TEXT_TEMPLATE.format(model=fw_model, filename=os.path.basename(fw_path))
                    send_email_with_attachment(sender, f"{fw_model} Latest Firmware", body, fw_path)
                    os.remove(fw_path)
                else:
                    send_email_plain(sender, f"{fw_model} Download Failed", f"Firmware download failed. Please try again later.")
            else:
                send_email_plain(sender, f"{fw_model} Not Found", f"No firmware found for {fw_model}.")
        else:
            print(f"{fw_model} 不在支持列表中, 当前列表: {supported_fw}")
            body = LIST_TEMPLATE.format(fw_list="\n".join(supported_fw))
            send_email_plain(sender, "Unknown FW Model", body)
    else:
        print("未识别型号，默认返回支持列表")
        body = LIST_TEMPLATE.format(fw_list="\n".join(supported_fw))
        send_email_plain(sender, "Firmware Support List", body)

def check_inbox_and_reply():
    print("正在检查收件箱...")
    try:
        mail = imaplib.IMAP4_SSL(IMAP_SERVER)
        mail.login(EMAIL_USER, EMAIL_PASS)
        mail.select('inbox')

        status, data = mail.search(None, 'UNSEEN SUBJECT "FW Request:"')
        if status != 'OK':
            print("搜索失败")
            return

        message_ids = data[0].split()
        print(f"找到未处理邮件数: {len(message_ids)}")

        for num in message_ids:
            typ, msg_data = mail.fetch(num, '(RFC822)')
            if typ != 'OK':
                continue
            msg = email.message_from_bytes(msg_data[0][1])
            subject = str(email.header.make_header(email.header.decode_header(msg['Subject'])))
            sender = email.utils.parseaddr(msg['From'])[1]
            handle_fw_request(subject, sender)
            mail.store(num, '+FLAGS', '\\Seen')
        mail.logout()
    except Exception as e:
        print(f"[ERROR] 邮箱处理失败: {e}")

if __name__ == '__main__':
    print("启动固件请求自动回复服务...")
    while True:
        check_inbox_and_reply()
        print(f"等待 {POLL_INTERVAL} 秒后再次检查...")
        time.sleep(POLL_INTERVAL)
