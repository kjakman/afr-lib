<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
		"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
<!-- 
  Script info: script: devscr, cmd: _send_ipn-session, template: tools/ipn, date: Mar. 27, 2009 19:44:26 PDT; country: US, language: en_US
  web version: 57.0-877624 branch: live-570_int
  content version: 57.0-877624 branch: live-570_int 
-->
<title>PayPal Sandbox - Instant Payment Notification (IPN) simulator</title>
<style type="text/css" media="all">@import "/css/dc.css";</style>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<meta name="description" content="Sandbox is a set of tools and resources to enable developers and merchants to develop eCommerce web sites and applications using PayPal web services." />
<meta name="keywords" content="Send, money, payments, credit, credit card, instant, money, financial services, api, soap, wsdl" />
<script type="text/javascript" src="/js/dc.js" language="JavaScript1.2"></script>
</head>
<body id="home-page">
<div id="page">
		
							<div id="content">
			
		
					  <img src="../../en_US/i/nav/hdr_main_image_sm.jpg" alt=" " width="569" height="63" />

			<H1>Instant Payment Notification (IPN) simulator</H1>
				
															<br />
		<div id="alert_container1">
			<div id="alert_img">
									<img src="/en_US/i/icon/icon_confirmation.gif" alt="Confirmation Icon" border="0" />
							</div>
			<div id="alert_content">
				<span class="message">IPN successfully sent.</span>

			</div>
		</div>
		
					
		
				
					<div id="subtext">Select from the transaction types supported to test the Instant Payment Notification (IPN) feature. Enter the URL of the webpage where you wish to receive IPNs, and the transaction type for this test.</div>
				

<form name="ipnform" method="post" action="ipn.php">
	<input type="hidden" id="set_cmd" name="ipn_tester" value="1" />
  <input type='submit' value='send'>
	<label for="ipnfields" class="mainhead">General information</label>
	<div id="ipnfields" class="display_heading">

		<!--	STARTS HERE - Mandatory Fields for IPN -->

			<div class="mandatory_row">
				<label for="notify_url" >IPN handler URL</label>
				<span class="field">
					<input type="text" class="textBox"  name="notify_url" id="notify_url" value="http://dev.apartments-for-rent.com/paypal/ipn.php" size="30" onkeyup="disableButton();" />
				</span>
			</div>

		  	<div class="mandatory_row">
				<label for="ipn_type">Transaction type</label>

				<span class="field">
				<select id="ipn_type" name="ipn_type" class="selectBox" onchange="ipntype_submit(this);">
					<option value="-1">- select -</option>
											<option  value="1">eCheck - pending</option>
											<option  value="2">eCheck - complete</option>
											<option  value="3">eCheck - declined</option>
											<option  value="4">Express Checkout</option>

											<option  value="5">Cart checkout</option>
											<option selected="selected" value="6">Web Accept</option>
											<option  value="7">Refund</option>
											<option  value="8">eBay checkout</option>
											<option  value="9">Reversal</option>
											<option  value="10">Canceled reversal</option>

									</select>
				<input type="hidden" id="set_cmd" name="cmd" value="" />
				</span>
			</div>
		<!-- ENDS HERE - Mandatory Fields for IPN -->

		<!-- STARTS HERE - Optional Fields for IPN -->
							<div id="ipn_details">
<!-- -------------------- -->
<!-- IPN Optional Fields -->

<!-- -------------------- -->

<div class="helptext">Default values provided in the fields below are valid. You may change any of these values, but your changed values will not be validated.</div>

<!-- STARTS HERE - PAYMENT INFORMATION -->

<label for="payment_info" class="internal">Payment information</label>
<div id="payment_info">
	<div class="internal">
		<label>payment_type</label>
		<div class="radiofield">

							<input type="radio"  name="payment_type" value="0"/><span class="optionText">echeck</span><br />
							<input type="radio" checked="checked" name="payment_type" value="1"/><span class="optionText">instant</span><br />
					</div>
	</div>
	
	<div class="internal">
		<label for="payment_date">payment_date</label>
		<span class="field">

			<input type="text" class="textBox" id="payment_date" name="payment_date" value="07:09:33 Apr. 23, 2009 PDT" size="30" />
		</span>
	</div>
	
	<div class="internal">
		<label for="payment_status">payment_status</label>
		<span class="field">
			<select name="payment_status" id="payment_status">
									<option  value="0">[- Please Select -]</option>

									<option  value="Canceled_Reversal">Canceled_Reversal</option>
									<option selected="selected" value="Completed">Completed</option>
									<option  value="Denied">Denied</option>
									<option  value="Expired">Expired</option>
									<option  value="Failed">Failed</option>
									<option  value="In-Progress">In-Progress</option>

									<option  value="Partially_Refunded">Partially_Refunded</option>
									<option  value="Pending">Pending</option>
									<option  value="Processed">Processed</option>
									<option  value="Refunded">Refunded</option>
									<option  value="Reversed">Reversed</option>
									<option  value="Voided">Voided</option>

							</select>
		</span>
	</div>

	<div class="external">
		<label for="pending_reason">pending_reason</label>
		<span class="field">
			<input type="text" class="textBox" id="pending_reason" name="pending_reason" value="" size="30" />
		</span>

	</div>
</div>

<!-- ENDS HERE - PAYMENT INFORMATION -->


<!-- STARTS HERE - BUYER INFORMATION -->

<label for="buyer_info" class="internal">Buyer information</label>
<div id="buyer_info">
	<div class="internal">
		<label>address_status</label>

		<div class="radiofield">
							<input type="radio"  name="address_status" value="0" /> unconfirmed <br />
							<input type="radio" checked="checked" name="address_status" value="1" /> confirmed <br />
					</div>
	</div>
	
	<div class="internal">
		<label>payer_status</label>

		<div class="radiofield">
							<input type="radio"  name="payer_status" value="0" /> unverified <br />
							<input type="radio" checked="checked" name="payer_status" value="1" /> verified <br />
					</div>
	</div>

	<div class="internal">

		<label for="first_name">first_name</label>
		<span class="field">
			<input type="text" class="textBox" id="first_name" name="first_name" value="John" size="30" maxlength="64" />
		</span>
	</div>

	<div class="internal">
		<label for="last_name">last_name</label>

		<span class="field">
			<input type="text" class="textBox" id="last_name" name="last_name" value="Smith" size="30" maxlength="64" />
		</span>
	</div>

	<div class="internal">
		<label for="payer_email">payer_email</label>
		<span class="field">
			<input type="text" class="textBox" id="payer_email" name="payer_email" value="buyer@paypalsandbox.com" size="30" maxlength="127" />

		</span>
	</div>

	<div class="internal">
		<label for="payer_id">payer_id</label>
		<span class="field">
			<input type="text" class="textBox" id="payer_id" name="payer_id" value="TESTBUYERID01" size="30" maxlength="13" />
		</span>
	</div>

	<div class="internal">
		<label for="address_name">address_name</label>
		<span class="field">
			<input type="text" class="textBox" id="address_name" name="address_name" value="John Smith" size="30" maxlength="128" />
		</span>
	</div>

	<div class="internal">

		<label for="address_country">address_country</label>
		<span class="field">
			<select name="address_country" id="address_country">
									<option  value="0">[- Please Select -]</option>
									<option  value="1">Albania</option>
									<option  value="2">Algeria</option>
									<option  value="3">Andorra</option>

									<option  value="4">Angola</option>
									<option  value="5">Anguilla</option>
									<option  value="6">Antigua and Barbuda</option>
									<option  value="7">Argentina</option>
									<option  value="8">Armenia</option>
									<option  value="9">Aruba</option>

									<option  value="10">Australia</option>
									<option  value="11">Austria</option>
									<option  value="12">Azerbaijan Republic</option>
									<option  value="13">Bahamas</option>
									<option  value="14">Bahrain</option>
									<option  value="15">Barbados</option>

									<option  value="16">Belgium</option>
									<option  value="17">Belize</option>
									<option  value="18">Benin</option>
									<option  value="19">Bermuda</option>
									<option  value="20">Bhutan</option>
									<option  value="21">Bolivia</option>

									<option  value="22">Bosnia and Herzegovina</option>
									<option  value="23">Botswana</option>
									<option  value="24">Brazil</option>
									<option  value="25">British Virgin Islands</option>
									<option  value="26">Brunei</option>
									<option  value="27">Bulgaria</option>

									<option  value="28">Burkina Faso</option>
									<option  value="29">Burundi</option>
									<option  value="30">Cambodia</option>
									<option  value="31">Canada</option>
									<option  value="32">Cape Verde</option>
									<option  value="33">Cayman Islands</option>

									<option  value="34">Chad</option>
									<option  value="35">Chile</option>
									<option  value="36">China Worldwide</option>
									<option  value="37">Colombia</option>
									<option  value="38">Comoros</option>
									<option  value="39">Cook Islands</option>

									<option  value="40">Costa Rica</option>
									<option  value="41">Croatia</option>
									<option  value="42">Cyprus</option>
									<option  value="43">Czech Republic</option>
									<option  value="44">Democratic Republic of the Congo</option>
									<option  value="45">Denmark</option>

									<option  value="46">Djibouti</option>
									<option  value="47">Dominica</option>
									<option  value="48">Dominican Republic</option>
									<option  value="49">Ecuador</option>
									<option  value="50">El Salvador</option>
									<option  value="51">Eritrea</option>

									<option  value="52">Estonia</option>
									<option  value="53">Ethiopia</option>
									<option  value="54">Falkland Islands</option>
									<option  value="55">Faroe Islands</option>
									<option  value="56">Federated States of Micronesia</option>
									<option  value="57">Fiji</option>

									<option  value="58">Finland</option>
									<option  value="59">France</option>
									<option  value="60">French Guiana</option>
									<option  value="61">French Polynesia</option>
									<option  value="62">Gabon Republic</option>
									<option  value="63">Gambia</option>

									<option  value="64">Germany</option>
									<option  value="65">Gibraltar</option>
									<option  value="66">Greece</option>
									<option  value="67">Greenland</option>
									<option  value="68">Grenada</option>
									<option  value="69">Guadeloupe</option>

									<option  value="70">Guatemala</option>
									<option  value="71">Guinea</option>
									<option  value="72">Guinea Bissau</option>
									<option  value="73">Guyana</option>
									<option  value="74">Honduras</option>
									<option  value="75">Hong Kong</option>

									<option  value="76">Hungary</option>
									<option  value="77">Iceland</option>
									<option  value="78">India</option>
									<option  value="79">Indonesia</option>
									<option  value="80">Ireland</option>
									<option  value="81">Israel</option>

									<option  value="82">Italy</option>
									<option  value="83">Jamaica</option>
									<option  value="84">Japan</option>
									<option  value="85">Jordan</option>
									<option  value="86">Kazakhstan</option>
									<option  value="87">Kenya</option>

									<option  value="88">Kiribati</option>
									<option  value="89">Kuwait</option>
									<option  value="90">Kyrgyzstan</option>
									<option  value="91">Laos</option>
									<option  value="92">Latvia</option>
									<option  value="93">Lesotho</option>

									<option  value="94">Liechtenstein</option>
									<option  value="95">Lithuania</option>
									<option  value="96">Luxembourg</option>
									<option  value="97">Madagascar</option>
									<option  value="98">Malawi</option>
									<option  value="99">Malaysia</option>

									<option  value="100">Maldives</option>
									<option  value="101">Mali</option>
									<option  value="102">Malta</option>
									<option  value="103">Marshall Islands</option>
									<option  value="104">Martinique</option>
									<option  value="105">Mauritania</option>

									<option  value="106">Mauritius</option>
									<option  value="107">Mayotte</option>
									<option  value="108">Mexico</option>
									<option  value="109">Mongolia</option>
									<option  value="110">Montserrat</option>
									<option  value="111">Morocco</option>

									<option  value="112">Mozambique</option>
									<option  value="113">Namibia</option>
									<option  value="114">Nauru</option>
									<option  value="115">Nepal</option>
									<option  value="116">Netherlands</option>
									<option  value="117">Netherlands Antilles</option>

									<option  value="118">New Caledonia</option>
									<option  value="119">New Zealand</option>
									<option  value="120">Nicaragua</option>
									<option  value="121">Niger</option>
									<option  value="122">Niue</option>
									<option  value="123">Norfolk Island</option>

									<option  value="124">Norway</option>
									<option  value="125">Oman</option>
									<option  value="126">Palau</option>
									<option  value="127">Panama</option>
									<option  value="128">Papua New Guinea</option>
									<option  value="129">Peru</option>

									<option  value="130">Philippines</option>
									<option  value="131">Pitcairn Islands</option>
									<option  value="132">Poland</option>
									<option  value="133">Portugal</option>
									<option  value="134">Qatar</option>
									<option  value="135">Republic of the Congo</option>

									<option  value="136">Reunion</option>
									<option  value="137">Romania</option>
									<option  value="138">Russia</option>
									<option  value="139">Rwanda</option>
									<option  value="140">Saint Vincent and the Grenadines</option>
									<option  value="141">Samoa</option>

									<option  value="142">San Marino</option>
									<option  value="143">S?o Tom? and Pr?ncipe</option>
									<option  value="144">Saudi Arabia</option>
									<option  value="145">Senegal</option>
									<option  value="146">Seychelles</option>
									<option  value="147">Sierra Leone</option>

									<option  value="148">Singapore</option>
									<option  value="149">Slovakia</option>
									<option  value="150">Slovenia</option>
									<option  value="151">Solomon Islands</option>
									<option  value="152">Somalia</option>
									<option  value="153">South Africa</option>

									<option  value="154">South Korea</option>
									<option  value="155">Spain</option>
									<option  value="156">Sri Lanka</option>
									<option  value="157">St. Helena</option>
									<option  value="158">St. Kitts and Nevis</option>
									<option  value="159">St. Lucia</option>

									<option  value="160">St. Pierre and Miquelon</option>
									<option  value="161">Suriname</option>
									<option  value="162">Svalbard and Jan Mayen Islands</option>
									<option  value="163">Swaziland</option>
									<option  value="164">Sweden</option>
									<option  value="165">Switzerland</option>

									<option  value="166">Taiwan</option>
									<option  value="167">Tajikistan</option>
									<option  value="168">Tanzania</option>
									<option  value="169">Thailand</option>
									<option  value="170">Togo</option>
									<option  value="171">Tonga</option>

									<option  value="172">Trinidad and Tobago</option>
									<option  value="173">Tunisia</option>
									<option  value="174">Turkey</option>
									<option  value="175">Turkmenistan</option>
									<option  value="176">Turks and Caicos Islands</option>
									<option  value="177">Tuvalu</option>

									<option  value="178">Uganda</option>
									<option  value="179">Ukraine</option>
									<option  value="180">United Arab Emirates</option>
									<option  value="181">United Kingdom</option>
									<option selected="selected" value="182">United States</option>
									<option  value="183">Uruguay</option>

									<option  value="184">Vanuatu</option>
									<option  value="185">Vatican City State</option>
									<option  value="186">Venezuela</option>
									<option  value="187">Vietnam</option>
									<option  value="188">Wallis and Futuna Islands</option>
									<option  value="189">Yemen</option>

									<option  value="190">Zambia</option>
							</select>
		</span>
	</div>

	<div class="internal">
		<label for="address_country_code">address_country_code</label>
		<span class="field">

			<select name="address_country_code" id="address_country_code">
									<option  value="0">[- Please Select -]</option>
									<option  value="1">AL</option>
									<option  value="2">DZ</option>
									<option  value="3">AD</option>
									<option  value="4">AO</option>

									<option  value="5">AI</option>
									<option  value="6">AG</option>
									<option  value="7">AR</option>
									<option  value="8">AM</option>
									<option  value="9">AW</option>
									<option  value="10">AU</option>

									<option  value="11">AT</option>
									<option  value="12">AZ</option>
									<option  value="13">BS</option>
									<option  value="14">BH</option>
									<option  value="15">BB</option>
									<option  value="16">BE</option>

									<option  value="17">BZ</option>
									<option  value="18">BJ</option>
									<option  value="19">BM</option>
									<option  value="20">BT</option>
									<option  value="21">BO</option>
									<option  value="22">BA</option>

									<option  value="23">BW</option>
									<option  value="24">BR</option>
									<option  value="25">VG</option>
									<option  value="26">BN</option>
									<option  value="27">BG</option>
									<option  value="28">BF</option>

									<option  value="29">BI</option>
									<option  value="30">KH</option>
									<option  value="31">CA</option>
									<option  value="32">CV</option>
									<option  value="33">KY</option>
									<option  value="34">TD</option>

									<option  value="35">CL</option>
									<option  value="36">CN</option>
									<option  value="37">CO</option>
									<option  value="38">KM</option>
									<option  value="39">CK</option>
									<option  value="40">CR</option>

									<option  value="41">HR</option>
									<option  value="42">CY</option>
									<option  value="43">CZ</option>
									<option  value="44">CD</option>
									<option  value="45">DK</option>
									<option  value="46">DJ</option>

									<option  value="47">DM</option>
									<option  value="48">DO</option>
									<option  value="49">EC</option>
									<option  value="50">SV</option>
									<option  value="51">ER</option>
									<option  value="52">EE</option>

									<option  value="53">ET</option>
									<option  value="54">FK</option>
									<option  value="55">FO</option>
									<option  value="56">FM</option>
									<option  value="57">FJ</option>
									<option  value="58">FI</option>

									<option  value="59">FR</option>
									<option  value="60">GF</option>
									<option  value="61">PF</option>
									<option  value="62">GA</option>
									<option  value="63">GM</option>
									<option  value="64">DE</option>

									<option  value="65">GI</option>
									<option  value="66">GR</option>
									<option  value="67">GL</option>
									<option  value="68">GD</option>
									<option  value="69">GP</option>
									<option  value="70">GT</option>

									<option  value="71">GN</option>
									<option  value="72">GW</option>
									<option  value="73">GY</option>
									<option  value="74">HN</option>
									<option  value="75">HK</option>
									<option  value="76">HU</option>

									<option  value="77">IS</option>
									<option  value="78">IN</option>
									<option  value="79">ID</option>
									<option  value="80">IE</option>
									<option  value="81">IL</option>
									<option  value="82">IT</option>

									<option  value="83">JM</option>
									<option  value="84">JP</option>
									<option  value="85">JO</option>
									<option  value="86">KZ</option>
									<option  value="87">KE</option>
									<option  value="88">KI</option>

									<option  value="89">KW</option>
									<option  value="90">KG</option>
									<option  value="91">LA</option>
									<option  value="92">LV</option>
									<option  value="93">LS</option>
									<option  value="94">LI</option>

									<option  value="95">LT</option>
									<option  value="96">LU</option>
									<option  value="97">MG</option>
									<option  value="98">MW</option>
									<option  value="99">MY</option>
									<option  value="100">MV</option>

									<option  value="101">ML</option>
									<option  value="102">MT</option>
									<option  value="103">MH</option>
									<option  value="104">MQ</option>
									<option  value="105">MR</option>
									<option  value="106">MU</option>

									<option  value="107">YT</option>
									<option  value="108">MX</option>
									<option  value="109">MN</option>
									<option  value="110">MS</option>
									<option  value="111">MA</option>
									<option  value="112">MZ</option>

									<option  value="113">NA</option>
									<option  value="114">NR</option>
									<option  value="115">NP</option>
									<option  value="116">NL</option>
									<option  value="117">AN</option>
									<option  value="118">NC</option>

									<option  value="119">NZ</option>
									<option  value="120">NI</option>
									<option  value="121">NE</option>
									<option  value="122">NU</option>
									<option  value="123">NF</option>
									<option  value="124">NO</option>

									<option  value="125">OM</option>
									<option  value="126">PW</option>
									<option  value="127">PA</option>
									<option  value="128">PG</option>
									<option  value="129">PE</option>
									<option  value="130">PH</option>

									<option  value="131">PN</option>
									<option  value="132">PL</option>
									<option  value="133">PT</option>
									<option  value="134">QA</option>
									<option  value="135">CG</option>
									<option  value="136">RE</option>

									<option  value="137">RO</option>
									<option  value="138">RU</option>
									<option  value="139">RW</option>
									<option  value="140">VC</option>
									<option  value="141">WS</option>
									<option  value="142">SM</option>

									<option  value="143">ST</option>
									<option  value="144">SA</option>
									<option  value="145">SN</option>
									<option  value="146">SC</option>
									<option  value="147">SL</option>
									<option  value="148">SG</option>

									<option  value="149">SK</option>
									<option  value="150">SI</option>
									<option  value="151">SB</option>
									<option  value="152">SO</option>
									<option  value="153">ZA</option>
									<option  value="154">KP</option>

									<option  value="155">ES</option>
									<option  value="156">LK</option>
									<option  value="157">SH</option>
									<option  value="158">KN</option>
									<option  value="159">LC</option>
									<option  value="160">PM</option>

									<option  value="161">SR</option>
									<option  value="162">SJ</option>
									<option  value="163">SZ</option>
									<option  value="164">SE</option>
									<option  value="165">CH</option>
									<option  value="166">TW</option>

									<option  value="167">TJ</option>
									<option  value="168">TZ</option>
									<option  value="169">TH</option>
									<option  value="170">TG</option>
									<option  value="171">TO</option>
									<option  value="172">TT</option>

									<option  value="173">TN</option>
									<option  value="174">TR</option>
									<option  value="175">TM</option>
									<option  value="176">TC</option>
									<option  value="177">TV</option>
									<option  value="178">UG</option>

									<option  value="179">UA</option>
									<option  value="180">AE</option>
									<option  value="181">GB</option>
									<option selected="selected" value="182">US</option>
									<option  value="183">UY</option>
									<option  value="184">VU</option>

									<option  value="185">VA</option>
									<option  value="186">VE</option>
									<option  value="187">VN</option>
									<option  value="188">WF</option>
									<option  value="189">YE</option>
									<option  value="190">ZM</option>

							</select>
		</span>
	</div>

	<div class="internal">
		<label for="address_zip">address_zip</label>
		<span class="field">
			<input type="text" class="textBox" id="address_zip" name="address_zip" value="95131" size="30" maxlength="20" />
		</span>

	</div>

	<div class="internal">
		<label for="address_state">address_state</label>
		<span class="field">
			<input type="text" class="textBox" id="address_state" name="address_state" value="CA" size="30" maxlength="40"  />
		</span>
	</div>

	<div class="internal">
		<label for="address_city">address_city</label>
		<span class="field">
			<input type="text" class="textBox" id="address_city" name="address_city" value="San Jose" size="30" maxlength="40" />
		</span>
	</div>

	<div class="internal">
		<label for="address_street">address_street</label>

		<span class="field">
			<input type="text" class="textBox" id="address_street" name="address_street" value="123, any street" size="30" maxlength="200"  />
		</span>
	</div>
</div>

<!-- ENDS HERE - BUYER INFORMATION -->


<!-- STARTS HERE - BASIC INFORMATION -->

<label for="basic_info" class="internal">Basic information</label>

<div id="basic_info">
	<div class="internal">
		<label for="business">business</label>
		<span class="field">
			<input type="text" class="textBox" id="business" name="business" value="seller@paypalsandbox.com" size="30" maxlength="127" />
		</span>
	</div>
	
	<div class="internal">
		<label for="receiver_email">receiver_email</label>

		<span class="field">
			<input type="text" class="textBox" id="receiver_email" name="receiver_email" value="test@kjakman.com" size="30" maxlength="127" />
		</span>
	</div>

	<div class="internal">
		<label for="receiver_id">receiver_id</label>
		<span class="field">
			<input type="text" class="textBox" id="receiver_id" name="receiver_id" value="TESTSELLERID1" size="30" maxlength="13" />

		</span>
	</div>
	
	<div class="internal">
		<label for="residence_country">residence_country</label>
		<span class="field">
			<select name="residence_country" id="residence_country">
									<option  value="0">[- Please Select -]</option>
									<option  value="1">AL</option>

									<option  value="2">DZ</option>
									<option  value="3">AD</option>
									<option  value="4">AO</option>
									<option  value="5">AI</option>
									<option  value="6">AG</option>
									<option  value="7">AR</option>

									<option  value="8">AM</option>
									<option  value="9">AW</option>
									<option  value="10">AU</option>
									<option  value="11">AT</option>
									<option  value="12">AZ</option>
									<option  value="13">BS</option>

									<option  value="14">BH</option>
									<option  value="15">BB</option>
									<option  value="16">BE</option>
									<option  value="17">BZ</option>
									<option  value="18">BJ</option>
									<option  value="19">BM</option>

									<option  value="20">BT</option>
									<option  value="21">BO</option>
									<option  value="22">BA</option>
									<option  value="23">BW</option>
									<option  value="24">BR</option>
									<option  value="25">VG</option>

									<option  value="26">BN</option>
									<option  value="27">BG</option>
									<option  value="28">BF</option>
									<option  value="29">BI</option>
									<option  value="30">KH</option>
									<option  value="31">CA</option>

									<option  value="32">CV</option>
									<option  value="33">KY</option>
									<option  value="34">TD</option>
									<option  value="35">CL</option>
									<option  value="36">CN</option>
									<option  value="37">CO</option>

									<option  value="38">KM</option>
									<option  value="39">CK</option>
									<option  value="40">CR</option>
									<option  value="41">HR</option>
									<option  value="42">CY</option>
									<option  value="43">CZ</option>

									<option  value="44">CD</option>
									<option  value="45">DK</option>
									<option  value="46">DJ</option>
									<option  value="47">DM</option>
									<option  value="48">DO</option>
									<option  value="49">EC</option>

									<option  value="50">SV</option>
									<option  value="51">ER</option>
									<option  value="52">EE</option>
									<option  value="53">ET</option>
									<option  value="54">FK</option>
									<option  value="55">FO</option>

									<option  value="56">FM</option>
									<option  value="57">FJ</option>
									<option  value="58">FI</option>
									<option  value="59">FR</option>
									<option  value="60">GF</option>
									<option  value="61">PF</option>

									<option  value="62">GA</option>
									<option  value="63">GM</option>
									<option  value="64">DE</option>
									<option  value="65">GI</option>
									<option  value="66">GR</option>
									<option  value="67">GL</option>

									<option  value="68">GD</option>
									<option  value="69">GP</option>
									<option  value="70">GT</option>
									<option  value="71">GN</option>
									<option  value="72">GW</option>
									<option  value="73">GY</option>

									<option  value="74">HN</option>
									<option  value="75">HK</option>
									<option  value="76">HU</option>
									<option  value="77">IS</option>
									<option  value="78">IN</option>
									<option  value="79">ID</option>

									<option  value="80">IE</option>
									<option  value="81">IL</option>
									<option  value="82">IT</option>
									<option  value="83">JM</option>
									<option  value="84">JP</option>
									<option  value="85">JO</option>

									<option  value="86">KZ</option>
									<option  value="87">KE</option>
									<option  value="88">KI</option>
									<option  value="89">KW</option>
									<option  value="90">KG</option>
									<option  value="91">LA</option>

									<option  value="92">LV</option>
									<option  value="93">LS</option>
									<option  value="94">LI</option>
									<option  value="95">LT</option>
									<option  value="96">LU</option>
									<option  value="97">MG</option>

									<option  value="98">MW</option>
									<option  value="99">MY</option>
									<option  value="100">MV</option>
									<option  value="101">ML</option>
									<option  value="102">MT</option>
									<option  value="103">MH</option>

									<option  value="104">MQ</option>
									<option  value="105">MR</option>
									<option  value="106">MU</option>
									<option  value="107">YT</option>
									<option  value="108">MX</option>
									<option  value="109">MN</option>

									<option  value="110">MS</option>
									<option  value="111">MA</option>
									<option  value="112">MZ</option>
									<option  value="113">NA</option>
									<option  value="114">NR</option>
									<option  value="115">NP</option>

									<option  value="116">NL</option>
									<option  value="117">AN</option>
									<option  value="118">NC</option>
									<option  value="119">NZ</option>
									<option  value="120">NI</option>
									<option  value="121">NE</option>

									<option  value="122">NU</option>
									<option  value="123">NF</option>
									<option  value="124">NO</option>
									<option  value="125">OM</option>
									<option  value="126">PW</option>
									<option  value="127">PA</option>

									<option  value="128">PG</option>
									<option  value="129">PE</option>
									<option  value="130">PH</option>
									<option  value="131">PN</option>
									<option  value="132">PL</option>
									<option  value="133">PT</option>

									<option  value="134">QA</option>
									<option  value="135">CG</option>
									<option  value="136">RE</option>
									<option  value="137">RO</option>
									<option  value="138">RU</option>
									<option  value="139">RW</option>

									<option  value="140">VC</option>
									<option  value="141">WS</option>
									<option  value="142">SM</option>
									<option  value="143">ST</option>
									<option  value="144">SA</option>
									<option  value="145">SN</option>

									<option  value="146">SC</option>
									<option  value="147">SL</option>
									<option  value="148">SG</option>
									<option  value="149">SK</option>
									<option  value="150">SI</option>
									<option  value="151">SB</option>

									<option  value="152">SO</option>
									<option  value="153">ZA</option>
									<option  value="154">KP</option>
									<option  value="155">ES</option>
									<option  value="156">LK</option>
									<option  value="157">SH</option>

									<option  value="158">KN</option>
									<option  value="159">LC</option>
									<option  value="160">PM</option>
									<option  value="161">SR</option>
									<option  value="162">SJ</option>
									<option  value="163">SZ</option>

									<option  value="164">SE</option>
									<option  value="165">CH</option>
									<option  value="166">TW</option>
									<option  value="167">TJ</option>
									<option  value="168">TZ</option>
									<option  value="169">TH</option>

									<option  value="170">TG</option>
									<option  value="171">TO</option>
									<option  value="172">TT</option>
									<option  value="173">TN</option>
									<option  value="174">TR</option>
									<option  value="175">TM</option>

									<option  value="176">TC</option>
									<option  value="177">TV</option>
									<option  value="178">UG</option>
									<option  value="179">UA</option>
									<option  value="180">AE</option>
									<option  value="181">GB</option>

									<option selected="selected" value="182">US</option>
									<option  value="183">UY</option>
									<option  value="184">VU</option>
									<option  value="185">VA</option>
									<option  value="186">VE</option>
									<option  value="187">VN</option>

									<option  value="188">WF</option>
									<option  value="189">YE</option>
									<option  value="190">ZM</option>
							</select>
		</span>
	</div>
	
	<div class="internal">

		<label for="item_name">item_name</label>
		<span class="field">
			<input type="text" class="textBox" id="item_name" name="item_name" value="something" size="30" maxlength="127" />
		</span>
	</div>

	<div class="internal">
		<label for="item_number">item_number</label>

		<span class="field">
			<input type="text" class="textBox" id="item_number" name="item_number" value="AK-1234" size="30" maxlength="127" />
		</span>
	</div>

	<div class="external">
		<label for="item_name1">item_name1</label>
		<span class="field">
			<input type="text" class="textBox" id="item_name1" name="item_name1" value="" size="30" maxlength="127" />

		</span>
	</div>

	<div class="external">
		<label for="item_number1">item_number1</label>
		<span class="field">
			<input type="text" class="textBox" id="item_number1" name="item_number1" value="" size="30" maxlength="127" />
		</span>
	</div>

	<div class="internal">
		<label for="quantity">quantity</label>
		<span class="field">
			<input type="text" class="textBox" id="quantity" name="quantity" value="1" size="30" />
		</span>
	</div>

	<div class="external">

		<label for="quantity1">quantity1</label>
		<span class="field">
			<input type="text" class="textBox" id="quantity1" name="quantity1" value="" size="30" />
		</span>
	</div>

	<div class="internal">
		<label for="shipping">shipping</label>

		<span class="field">
			<input type="text" class="textBox" id="shipping" name="shipping" value="3.04" size="30" />
		</span>
	</div>
	
	<div class="internal">
		<label for="tax">tax</label>
		<span class="field">
			<input type="text" class="textBox" id="tax" name="tax" value="2.02" size="30" />

		</span>
	</div>
</div>

<!-- ENDS HERE - BASIC INFORMATION -->


<!-- STARTS HERE - CURRENCY AND CURRENCY EXCHANGE -->

<label for="currrency_exchange" class="internal">Currency and currrency exchange</label>
<div id="currrency_exchange">
	<div class="internal">
		<label for="mc_currency">mc_currency</label>

		<span class="field">
			<input type="mc_currency" class="textBox" id="mc_currency" name="mc_currency" value="EUR" size="3" />
		</span>
	</div>

	<div class="internal">
		<label for="mc_fee">mc_fee</label>
		<span class="field">
			<input type="text" class="textBox" id="mc_fee" name="mc_fee" value="0.44" size="30" />
		</span>

	</div>

	<div class="internal">
		<label for="mc_gross">mc_gross</label>
		<span class="field">
			<input type="text" class="textBox" id="mc_gross" name="mc_gross" value="12.34" size="30" />
		</span>
	</div>

	<div class="internal">
		<label for="mc_gross1">mc_gross1</label>
		<span class="field">
			<input type="text" class="textBox" id="mc_gross1" name="mc_gross1" value="9.34" size="30" />
		</span>
	</div>

	<div class="external">
		<label for="mc_handling">mc_handling</label>

		<span class="field">
			<input type="text" class="textBox" id="mc_handling" name="mc_handling" value="" size="30" />
		</span>
	</div>

	<div class="external">
		<label for="mc_handling1">mc_handling1</label>
		<span class="field">
			<input type="text" class="textBox" id="mc_handling1" name="mc_handling1" value="" size="30" />

		</span>
	</div>
	
	<div class="external">
		<label for="mc_shipping">mc_shipping</label>
		<span class="field">
			<input type="text" class="textBox" id="mc_shipping" name="mc_shipping" value="" size="30" />
		</span>
	</div>

	
	<div class="external">
		<label for="mc_shipping1">mc_shipping1</label>
		<span class="field">
			<input type="text" class="textBox" id="mc_shipping1" name="mc_shipping1" value="" size="30" />
		</span>
	</div>
</div>

<!-- ENDS HERE - CURRENCY AND CURRENCY EXCHANGE -->

<!-- STARTS HERE - TRANSACTION FIELDS -->

<label for="transaction_fields" class="internal">Transaction fields</label>
<div id="transaction_fields">
	<div class="internal">
		<label for="txn_type">txn_type</label>
		<span class="field">
			<input type="text" class="textBox" id="txn_type" name="txn_type" value="web_accept" size="30" />
		</span>

	</div>

	<div class="internal">
		<label for="txn_id">txn_id</label>
		<span class="field">
			<input type="text" class="textBox" id="txn_id" name="txn_id" value="<?=rand(10000, 100000)?>" size="30" maxlength="17" />
		</span>
	</div>

	<div class="external">
		<label for="parent_txn_id">parent_txn_id</label>
		<span class="field">
			<input type="text" class="textBox" id="parent_txn_id" name="parent_txn_id" value="" size="30" maxlength="17" />
		</span>
	</div>
	
	<div class="internal">
		<label for="notify_version">notify_version</label>

		<span class="read_only">
			2.1&nbsp;
			<input type="hidden" id="notify_version" name="notify_version " value="2.1" />
		</span>
	</div>
</div>

<!-- ENDS HERE - TRANSACTION FIELDS -->


<!-- STARTS HERE - EBAY AUCTION -->

<label for="ebay_auction" class="external">eBay auction</label>
<div id="ebay_auction">
	<div class="external">
		<label for="auction_buyer_id">auction_buyer_id</label>
		<span class="field">
			<input type="text" class="textBox" id="auction_buyer_id" name="auction_buyer_id" value="" size="30" maxlength="64" />
		</span>
	</div>

	<div class="external">
		<label for="auction_closing_date">auction_closing_date</label>
		<span class="field">
			<input type="text" class="textBox" id="auction_closing_date" name="auction_closing_date" value="" size="30" />
		</span>
	</div>
	
	<div class="external">
		<label for="for_auction">for_auction</label>

		<span class="field">
			<input type="text" class="textBox" id="for_auction" name="for_auction" value="" size="30" />
		</span>
	</div>
</div>

<!-- ENDS HERE - EBAY AUCTION -->


<!-- STARTS HERE - REFUNDS/REVERSALS -->

<label for="refunds_reversals" class="external">Refunds/reversals</label>

<div id="refunds_reversals">
	<div class="external">
		<label for="reason_code">reason_code</label>
		<span class="field">
			<input type="text" class="textBox" id="reason_code" name="reason_code" value="" size="30" />
		</span>
	</div>

	<div class="external">

		<label for="receipt_ID">receipt_ID</label>
		<span class="field">
			<input type="text" class="textBox" id="receipt_ID" name="receipt_ID" value="" size="30" />
		</span>
	</div>
</div>

<!-- ENDS HERE - REFUNDS/REVERSALS -->


<!-- STARTS HERE - ADVANCED/CUSTOM -->

<label for="advanced_custom" class="internal">Advanced and custom information</label>
<div id="advanced_custom">
	<div class="internal">
		<label for="custom">custom</label>
		<span class="field">
			<input type="text" class="textBox" id="custom" name="custom" value="res_id=27611,inv_id=2067" size="30" maxlength="255" />

		</span>
	</div>

	<div class="external">
		<label for="invoice">invoice</label>
		<span class="field">
			<input type="text" class="textBox" id="invoice" name="invoice" value="12345-01" size="30" maxlength="127" />
		</span>
	</div>
</div>

<!-- ENDS HERE - ADVANCED/CUSTOM -->

<!-- Show All Fields Check box -->

<div class="internal">
	<label for="show_fields" class="checkbox">Show all fields</label>
	<span class="field">
		<input type="checkbox" name="show_fields" id="show_fields" onclick="checkSubmit(this)"/>
	</span>
</div></div>
					<!-- ENDS HERE - Optional Fields for IPN -->

		<!-- STARTS HERE - IPN Buttons -->
																											
						<div id="buttons" class="footerDotLine">


	<!--	<hr id="titleWidthExpand"/>
	-->
		
			
</div>
		<!-- ENDS HERE - IPN Buttons -->

	</div>
</form>

	<div style="display:block; height:200px"></div>
	</div>
	 <!--
<div id="dcfooter">&nbsp;&nbsp;Copyright &#169; 1999-2008 PayPal. All rights reserved. <a style="color:#cccccc;">|</a> <a href="#" onclick="openUA('devscr?cmd=legal/ua_pop-outside'); return false;">User Agreement</a> <a style="color:#cccccc;">|</a> <a href="#" onclick="openUA('devscr?cmd=legal/privacy_pop-outside'); return false;">Privacy Policy</a></div>

-->

<hr class="divider"/>
	<div id="footer">
		<div class="nav-footer">
			<ul>

				<li class="first"><a href="https://www.paypal.com/webscr?cmd=_display-fees-outside" title="Fees">Fees</a></li>
				<li><a href="https://www.paypal.com/webscr?cmd=p/gen/ua/policy_privacy-outside" title="Privacy">Privacy</a></li>
				<li><a href="https://www.paypal.com/webscr?cmd=_security-center-outside" title="Security Center">Security Center</a></li>
				<li><a href="https://www.paypal.com/IntegrationCenter/ic_contact-support.html" title="Contact Us">Contact Us</a></li>
				<li><a href="https://www.paypal.com/webscr?cmd=p/gen/ua/ua-outside" title="Legal Agreements">Legal Agreements</a></li>
				<li><a href="/en_US/pdf/PP_Sandbox_UserAgreement.pdf" target="_blank" title="User Agreement">User Agreement</a></li>

			</ul>
			<p class="company">PayPal, an eBay Company</p>
		</div>
		<div class="copyright">
			<p>Copyright &copy; 1999-2009 PayPal. All rights reserved.</p>
			<p><a href="https://www.paypal.com/webscr?cmd=p/gen/fdic-outside" title="Information about FDIC pass-through insurance">Information about FDIC pass-through insurance</a></p>

		</div>
	 </div>
</div>
</body>
</html>