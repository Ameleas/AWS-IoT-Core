export const handler = async (event) => {
  console.log("Received Event: ", event);

  const webhookUrl = process.env.WebhookURL;
  console.log("Webhook URL: ", webhookUrl);

  const discordMessage = {
    "content": event.eventType,
    "embeds": [
        {
            "title": event.clientId,
            "description":new Date(event.timestamp).toISOString(),
            "color": "45973"
        }
    ]
  };

  const r = await fetch(webhookUrl, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(discordMessage),
  });

  if (r.status === 204) {
    console.log("Discord webhook response: ", r)
  }
  else {
    console.error("Discord webhook error: ", r)
  }

  return;
};